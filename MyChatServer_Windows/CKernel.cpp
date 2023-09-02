#include "CKernel.h"
#include "packDef.h"
#include <time.h>
#include <list>
#include <string>
#include <random>
#include "TcpServer.h"

#pragma comment(lib,"MD5.lib")

#define NetProtocolMap(a) (m_netProtocolMap[a - _DEF_PROTOCOL_BASE - 1])

CKernel CKernel::m_kernel;
CKernel::CKernel() {
	m_pMediator = new TcpServerMediator;
	
	m_mysql = new CMySql;

	bindProtocolMap();
}
CKernel::~CKernel() {

}
//打开服务器
bool CKernel::startServer() {
	//1、打开网络
	if (!m_pMediator->OpenNet()) {
		cout << "打开网络失败" << endl;
		return false;
	}
	//2、连接mysql数据库
	if (!m_mysql->ConnectMySql("127.0.0.1", "root", "123456", "mychat")) {
		cout << "ConnectMySql error" << endl;
		return false;
	}
	else {
		cout << "ConnectMySql success" << endl;
	}
	return true;
}
//关闭服务器
void CKernel::closeServer() {
	//关闭网络
	if (m_pMediator) {
		m_pMediator->CloseNet();
		delete m_pMediator;
		m_pMediator = nullptr;
	}
	//与数据库断开连接
	if (m_mysql) {
		m_mysql->DisConnect();
		delete m_mysql;
		m_mysql = nullptr;
	}

	//清空节点
	m_mapIdToSocket.clear();

	m_lock.writeLock();
	auto ite = m_FileIdToInfo.begin();
	while (ite != m_FileIdToInfo.end()) {
		delete ite->second;
		ite->second = nullptr;
		m_FileIdToInfo.erase(ite);
	}
	m_FileIdToInfo.clear();
	m_lock.writeUnlock();
}

//处理接收所有客户端的数据
void CKernel::dealData(char* buf, int nLen, long lSend) {
	cout << "CKernel::dealData" << endl;
	// 拆包
	PackType type = *(PackType*)buf;
	
	if (_DEF_PROTOCOL_BASE < type && type <= _DEF_PROTOCOL_BASE + DEF_PROTOCOL_COUNT) {
		pfun fun = NetProtocolMap(type);
		(this->*fun)(buf, nLen, lSend);
	}

	//处理完后将资源回收
	delete[] buf;
	buf = nullptr;
}

//处理注册请求
void CKernel::dealRegisterRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_REGISTER_RQ* rq = (STRU_TCP_REGISTER_RQ*)buf;
	//cout << "tel:" << rq->tel << " password:" << rq->password;
	//创建注册回复
	STRU_TCP_REGISTER_RS rs;

	//检测手机号是否被注册过
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from mychat.t_user where tel = '%s';", rq->tel);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << "SelectMySql error" << endl;
	}
	if (lstSelect.size() > 0) {
		//已经被注册过
		rs.result = tel_is_exist;
	}
	else {
		//手机号没有被注册过
		rs.result = register_success;

		//将密码再与随机盐进行计算MD5哈希值
		string salt = GetSalt(DEF_SALT_SIZE);
		const char* password = m_md5.md5((salt + rq->password).c_str());
		//cout << rq->tel <<"  " << rq->password << "	" << password << "	" << salt << endl;
		//将注册信息写入数据库
		memset(sqlBuf, 0, 1024);
		sprintf_s(sqlBuf, "insert into mychat.t_user (tel, password, salt) value('%s','%s','%s');", rq->tel, password, salt.c_str());
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error" << endl;
		}
	}

	//发送注册回复
	if (!m_pMediator->SendData((char*)&rs, sizeof(rs), lSend)) {
		cout << "dealRegisterRq send Rs error" << endl;
	}
}

//处理登录请求
void CKernel::dealLoginRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_LOGIN_RQ* rq = (STRU_TCP_LOGIN_RQ*)buf;
	
	//创建登录回复包
	STRU_TCP_LOGIN_RS rs;
	
	//校验账号密码
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id,password,salt from mychat.t_user where tel = '%s';", rq->tel);
	if (!m_mysql->SelectMySql(sqlBuf, 3, lstSelect)) {
		cout << "SelectMySql error" << endl;
	}
	if (lstSelect.empty()) {
		//用户不存在
		rs.result = tel_error;
		//发送登录回复
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
	}
	else {
		//保存用户id
		int userId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		//检测是否已经登录
		TcpServer::m_threadLock.readLock();
		if (m_mapIdToSocket.find(userId) != m_mapIdToSocket.end()) {
			//已经登录，无法再进行登录
			rs.result = had_login;
			//发送登录回复
			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
			return;
		}
		TcpServer::m_threadLock.readUnlock();

		//提取随机盐与该用户密码相加
		string tempPassword = lstSelect.back() + rq->password;
		lstSelect.pop_back();

		//计算相加的MD5的值
		const char* password = m_md5.md5(tempPassword.c_str());
		cout << password << "   " << lstSelect.front() << endl;

		if (0 == strcmp(password, lstSelect.front().c_str())) {
			//密码正确
			rs.result = login_success;
			rs.userId = userId;

			//发送登录回复给该用户
			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

			//将该用户的sock保存至map
			TcpServer::m_threadLock.writeLock();
			m_mapIdToSocket[userId] = lSend;
			TcpServer::m_threadLock.writeUnlock();

			//获取好友列表 （群聊列表后续实现）
			GetAllFriendInfo(userId);
			GetAllGroupInfo(userId);

			//TODO:获取好友发送的文件
			GetAllFile(userId);
		}
		else {
			//密码错误
			rs.result = password_error;
			//发送登录回复
			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		}
	}
}
//处理聊天请求
void CKernel::dealChatRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_CHAT_RQ* rq = (STRU_TCP_CHAT_RQ*)buf;
	//通过用户id查询是否存在该好友
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select idB from mychat.t_friend where idA = %d and idB = %d;", rq->userId, rq->friendId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (!lstSelect.empty()) {
		int temp = rq->friendId;
		rq->friendId = rq->userId;
		rq->userId = temp;
		TcpServer::m_threadLock.readLock();
		if (m_mapIdToSocket.find(rq->userId) != m_mapIdToSocket.end()) {
			//存在该好友并且在线，将聊天请求转发给该好友
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[rq->userId]);
		}
		TcpServer::m_threadLock.readUnlock();
	}
}
//处理下线请求
void CKernel::dealOfflineRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_OFFLINE_RQ* rq = (STRU_TCP_OFFLINE_RQ*)buf;

	//将下线消息转发给所有的好友 （和群聊、这个功能后续添加）
	//通过用户id查询所有好友
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select idB from mychat.t_friend where idA = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	
	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		//获取好友id
		int friendId = atoi(lstSelect.front().c_str());
		//查看该好友是否在线
		if (m_mapIdToSocket.find(friendId) != m_mapIdToSocket.end()) {
			//将下线请求转发给该好友
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[friendId]);
		}
		lstSelect.pop_front();
	}
	lstSelect.clear();

	//断开与该用户的连接 关闭此线程
	closesocket(m_mapIdToSocket[rq->userId]);
	TcpServer::m_threadLock.readUnlock();

	//TODO:将未发送完的文件记录到数据库中
	sprintf_s(sqlBuf, "select fileId from t_file where sendId = %d and t_flag = 0;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	while (!lstSelect.empty()) {
		m_lock.readLock();
		if (m_FileIdToInfo.find(lstSelect.front()) != m_FileIdToInfo.end()) {
			STRU_FILE_INFO* info = m_FileIdToInfo[lstSelect.front()];
			if (info) {
				sprintf_s(sqlBuf, "update t_file set pos = %lld where fileId = '%s';", info->filePos, info->fileId);
				if (!m_mysql->UpdateMySql(sqlBuf)) {
					cout << __func__ << "error" << endl;
					m_lock.readUnlock();
					return;
				}
			}
		}
		lstSelect.pop_front();
		m_lock.readUnlock();
	}

	//将该用户的sock从IdToSockmap中删除
	TcpServer::m_threadLock.writeLock();
	m_mapIdToSocket.erase(rq->userId);
	TcpServer::m_threadLock.writeUnlock();

	//将该线程的sock从IdToSockmap中删除
	TcpServer::m_threadLock.Lock();
	TcpServer::m_mapThreadIdToSocket.erase(GetCurrentThreadId());
	TcpServer::m_threadLock.UnLock();
	//通过用户id查询所有群聊
	sprintf_s(sqlBuf, "select groupId from mychat.t_groupmember where memberId = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	STRU_TCP_GROUP_MEMBER_OFFLINE_RQ offlineRq;
	offlineRq.userId = rq->userId;
	//给每个群的在线用户发送群聊的下线请求
	while (!lstSelect.empty()) {
		int groupId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		offlineRq.groupId = groupId;
		//获取此群的所有成员 将下线请求发送给在线的成员
		list<string> lstMember;
		sprintf_s(sqlBuf, "select memberId from mychat.t_groupmember where groupId = %d;", groupId);
		if (!m_mysql->SelectMySql(sqlBuf, 1, lstMember)) {
			cout << __func__ << "error" << endl;
			return;
		}
		while (!lstMember.empty()) {
			int memberId = atoi(lstMember.front().c_str());
			lstMember.pop_front();
			TcpServer::m_threadLock.readLock();
			if (m_mapIdToSocket.find(memberId) != m_mapIdToSocket.end()) {
				//该成员在线 发送下线请求包
				m_pMediator->SendData((char*)&offlineRq, sizeof(offlineRq), m_mapIdToSocket[memberId]);
			}
			TcpServer::m_threadLock.readUnlock();
		}
	}
}

//处理添加好友请求
void CKernel::dealAddFriendRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_ADDFRIEND_RQ* rq = (STRU_TCP_ADDFRIEND_RQ*)buf;
	
	//查看用户是否存在
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id, name from mychat.t_user where tel = %s;", rq->friendTel);
	if (!m_mysql->SelectMySql(sqlBuf, 2, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		//该用户不存在
		STRU_TCP_ADDFRIEND_RS rs;
		rs.result = addfriend_not_exist;
		//发送回客户端
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	if (atoi(lstSelect.front().c_str()) == rq->userId) {
		//不能添加自己为好友
		STRU_TCP_ADDFRIEND_RS rs;
		rs.result = addfriend_mine;
		//发送回客户端
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}

	//strcpy(rq->userName, lstSelect.back().c_str());
	//查看用户是否在线
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(atoi(lstSelect.front().c_str())) == m_mapIdToSocket.end()) {
		//用户不在线
		STRU_TCP_ADDFRIEND_RS rs;
		rs.result = addfriend_offline;
		//发送回客户端
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	
	//用户在线，将请求打包发送给该用户
	m_pMediator->SendData(buf, nLen, m_mapIdToSocket[atoi(lstSelect.front().c_str())]);
	TcpServer::m_threadLock.readUnlock();
}
//处理添加好友回复
void CKernel::dealAddFriendRs(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_ADDFRIEND_RS* rs = (STRU_TCP_ADDFRIEND_RS*)buf;
	//查询这个人的名字
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name from mychat.t_user where id = %d;", rs->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	strcpy(rs->userName, lstSelect.front().c_str());
	//将添加好友回复转发给要添加的用户
	TcpServer::m_threadLock.readUnlock();
	m_pMediator->SendData(buf, nLen, m_mapIdToSocket[rs->friendId]);
	

	if (rs->result == addfriend_success) {
		//添加成功，将彼此的信息发送给对方
		STRU_TCP_FRIEND_INFO userInof;
		STRU_TCP_FRIEND_INFO friendInof;
		GetUserInfo(&userInof, rs->friendId);
		GetUserInfo(&friendInof, rs->userId);
		//添加成功，将好友关系写入数据库（写入两次，为了保证两次都写入成功或者失败，需要使用事务）
		char sqlBuf[1024] = "";
		sprintf_s(sqlBuf, "insert into t_friend (idA, idB) values (%d, %d)", rs->userId, rs->friendId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "插入数据库失败，sql:" << sqlBuf << endl;
			return;
		}
		sprintf_s(sqlBuf, "insert into t_friend (idA, idB) values (%d, %d)", rs->friendId, rs->userId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "插入数据库失败，sql:" << sqlBuf << endl;
			return;
		}

		m_pMediator->SendData((char*)&userInof, sizeof(userInof), lSend);
		m_pMediator->SendData((char*)&friendInof, sizeof(friendInof), m_mapIdToSocket[rs->friendId]);
	}
	TcpServer::m_threadLock.readUnlock();
}
//处理删除好友请求
void CKernel::dealDeleteFriendRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_DELETEFRIEND_RQ* rq = (STRU_TCP_DELETEFRIEND_RQ*)buf;

	STRU_TCP_DELETEFRIEND_RS rs;
	//查询是否存在好友关系
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select idB from t_friend where idA = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		rs.result = deletefriend_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	lstSelect.clear();

	//将好友关系表中的数据进行删除
	sprintf_s(sqlBuf, "delete from t_friend where idA = %d and idB = %d;", rq->userId, rq->friendId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << "删除数据库失败，sql:" << sqlBuf << endl;
		rs.result = deletefriend_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	sprintf_s(sqlBuf, "delete from t_friend where idA = %d and idB = %d;", rq->friendId, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << "删除数据库失败，sql:" << sqlBuf << endl;
		rs.result = deletefriend_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}

	//删除成功后发送删除好友请求给对方，把删除好友回复发送给本方
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(rq->friendId) != m_mapIdToSocket.end()) {
		m_pMediator->SendData(buf, nLen, m_mapIdToSocket[rq->friendId]);
	}
	TcpServer::m_threadLock.readUnlock();
	
	rs.result = deletefriend_success;
	rs.friendId = rq->friendId;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
}
//处理更新用户信息请求
void CKernel::dealUserUpdataRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_UPDATA_RQ* rq = (STRU_TCP_UPDATA_RQ*)buf;

	//查询是否存在该用户
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_user where id = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) return;
	//更新数据
	sprintf_s(sqlBuf, "update t_user set name = '%s',icon = %d, feeling = '%s' where id = %d;", 
		rq->userName, rq->iconId, rq->userFeeling,rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << " UpdateMySql" << endl;
		return;
	}

	//查询该用户的好友
	lstSelect.clear();
	sprintf_s(sqlBuf, "select idB from t_friend where idA = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		//将更新的内容发送给每个在线的好友
		int friendId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		if (m_mapIdToSocket.find(friendId) != m_mapIdToSocket.end()) {
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[friendId]);
		}
	}
	TcpServer::m_threadLock.readUnlock();

	//将更新的资料发送给每个群
	//通过用户id查询所有群聊
	sprintf_s(sqlBuf, "select groupId from mychat.t_groupmember where memberId = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	STRU_TCP_GROUOP_MEMBER_INFO info;
	GetGroupMemberInfo(&info, rq->userId);
	//给每个群的在线用户发送该用户更新的资料
	while (!lstSelect.empty()) {
		int groupId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		info.groupId = groupId;
		//获取此群的所有成员 将资料发送给在线的成员
		list<string> lstMember;
		sprintf_s(sqlBuf, "select memberId from mychat.t_groupmember where groupId = %d;", groupId);
		if (!m_mysql->SelectMySql(sqlBuf, 1, lstMember)) {
			cout << __func__ << "error" << endl;
			return;
		}
		while (!lstMember.empty()) {
			int memberId = atoi(lstMember.front().c_str());
			lstMember.pop_front();
			TcpServer::m_threadLock.readLock();
			if (m_mapIdToSocket.find(memberId) != m_mapIdToSocket.end()) {
				//该成员在线 发送下线请求包
				m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[memberId]);
			}
			TcpServer::m_threadLock.readUnlock();
		}
	}
}

//处理创建群聊请求
void CKernel::dealCreateGroupRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_CREATEGROUP_RQ* rq = (STRU_TCP_CREATEGROUP_RQ*)buf;
	
	//准备创建群聊回复包
	STRU_TCP_CREATEGROUP_RS rs;

	//在群聊信息表中添加一行，使用触发器同时在群成员列表中添加一行
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "insert into t_groupchat (name,userId) values ('%s',%d);", rq->groupName, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << endl;
		rs.result = creategroup_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	//创建成功 发送创建群聊回复给客户端
	rs.result = creategroup_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

	//将该用户的群聊的信息发送回客户端（id最大的那个为新创建的群聊）
	list<string> lstSelect;
	sprintf_s(sqlBuf, "select max(id) from t_groupchat where userId = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (!lstSelect.empty()) {
		STRU_TCP_GROUOP_INFO info;
		int groupId = atoi(lstSelect.front().c_str());
		GetGroupInfo(&info, groupId);
		m_pMediator->SendData((char*)&info, sizeof(info), lSend);

		GetAllGroupMemberInfo(groupId, rq->userId);
	}
}
//处理加入群聊请求
void CKernel::dealJoinGroupRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_JOINGROUP_RQ* rq = (STRU_TCP_JOINGROUP_RQ*)buf;
	
	//准备回复包
	STRU_TCP_JOINGROUP_RS rs;

	//根据群id查找是否存在该群
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_groupchat where id = %d;", rq->groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		//不存在该群
		rs.result = joingroup_not_exist;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	
	//发送加群请求回复
	rs.result = joingroup_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

	//插入群和该成员的关系表	使用存储过程
	sprintf_s(sqlBuf, "call joingroup(%d, %d);", rq->groupId, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << " error" << endl;
		return;
	}
	//将该群聊信息发送给该客户端和每个在线的群成员
	if (!lstSelect.empty()) {
		STRU_TCP_GROUOP_INFO info;
		int groupId = atoi(lstSelect.front().c_str());
		
		//属于更新群聊信息
		//发送群聊信息给所有在线的群成员
		GetGroupInfo(&info, groupId);
		sprintf_s(sqlBuf, "select memberId from t_groupmember where groupId = %d;", rq->groupId);
		if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
			cout << __func__ << "error" << endl;
			return;
		}

		TcpServer::m_threadLock.readLock();
		while (!lstSelect.empty()) {
			int userId = atoi(lstSelect.front().c_str());
			lstSelect.pop_front();
			if (m_mapIdToSocket.find(userId) != m_mapIdToSocket.end()) {
				m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[userId]);
			}
		}
		TcpServer::m_threadLock.readUnlock();

		//获取好友列表
		GetAllGroupMemberInfo(groupId, rq->userId);
	}
}
//处理更新群聊信息请求
void CKernel::dealGroupUpDataInfoRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_GROUOP_INFO* info = (STRU_TCP_GROUOP_INFO*)buf;
	
	//检测群聊id和用户id是否匹配
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_groupchat where id = %d and userId = %d;", info->groupId, info->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		//不匹配直接退出
		return;
	}
	//更新群聊信息
	sprintf_s(sqlBuf, "update t_groupchat set name = '%s', icon = %d where id = %d;", info->name, info->groupIconId, info->groupId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << " error" << endl;
		return;
	}
	lstSelect.clear();

	//发送群聊信息给所有在线的群成员
	GetGroupInfo(info, info->groupId);
	sprintf_s(sqlBuf, "select memberId from t_groupmember where groupId = %d;", info->groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		int userId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		if (m_mapIdToSocket.find(userId) != m_mapIdToSocket.end()) {
			m_pMediator->SendData((char*)info, sizeof(*info), m_mapIdToSocket[userId]);
		}
	}
	TcpServer::m_threadLock.readUnlock();
}
//处理解散群聊请求
void CKernel::dealGroupDeleteRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_GROUP_DELETE_RQ* rq = (STRU_TCP_GROUP_DELETE_RQ*)buf;

	//先查询数据库该用户是否为群主
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_groupchat where id = %d and userId = %d;", rq->groupId, rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		return;
	}
	lstSelect.clear();
	
	//先将该请求转发给在线的群成员、转发完后再删除信息
	sprintf_s(sqlBuf, "select memberId from t_groupmember where groupId = %d;", rq->groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		int memberId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		if (m_mapIdToSocket.find(memberId) != m_mapIdToSocket.end()) {
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[memberId]);
		}
	}
	TcpServer::m_threadLock.readUnlock();

	sprintf_s(sqlBuf, "delete from t_groupchat where id = %d;", rq->groupId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << endl;
		return;
	}
}
//处理退出群聊请求
void CKernel::dealGroupExitRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_GROUP_DELETE_RQ* rq = (STRU_TCP_GROUP_DELETE_RQ*)buf;

	//删除数据库中群聊成员表中的数据
	
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "delete from t_groupmember where groupId = %d and memberId = %d;", rq->groupId, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << endl;
		return;
	}

	//获取该群聊的成员列表 将该用户退出群聊的消息转发给所有在线的好友
	list<string> lstSelect;
	sprintf_s(sqlBuf, "select memberId from t_groupmember where groupId = %d;", rq->groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		int memberId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		if (m_mapIdToSocket.find(memberId) != m_mapIdToSocket.end()) {
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[memberId]);
		}
	}
	TcpServer::m_threadLock.readUnlock();
}
//处理群聊消息
void CKernel::dealGroupMsg(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_GROUP_CHAT_RQ* rq = (STRU_TCP_GROUP_CHAT_RQ*)buf;

	//查询该群的其他成员
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select memberId from t_groupmember where groupId = %d and memberId != %d;", rq->groupId, rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	//将消息转发给其他在线成员
	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		int memberId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		if (m_mapIdToSocket.find(memberId) != m_mapIdToSocket.end()) {
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[memberId]);
		}
	}
	TcpServer::m_threadLock.readUnlock();
}

//处理好友发送文件请求
void CKernel::dealFriendSendFileRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_SEND_FILE_RQ* rq = (STRU_TCP_SEND_FILE_RQ*)buf;
	STRU_TCP_SEND_FILE_RS rs;
	rs.friendId = rq->friendId;
	//创建好友用户的本地文件夹
	char path[DEF_FILE_NAME_SIZE] = "";
	sprintf(path, "D:/Server/%d", rq->friendId);
	if (!CreateDirectoryA(path, NULL) && GetLastError() != 183) {
		cout << "CreateDirectoryA error " << GetLastError()<< endl;
		rs.result = sendfile_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	//创建该文件 
	sprintf(path, "D:/Server/%d/%s", rq->friendId, rq->fileName);
	FILE* pFile = CreateLargeFile(path, rq->fileSize);

	if (pFile == nullptr) {
		cout << "CreateLargeFile error" << GetLastError() << endl;
		rs.result = sendfile_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}

	//查询数据库中是否有记录
	list<string> Selectlist;
	char sqlBuf[2048] = "";
	sprintf(sqlBuf, "select pos from t_file where fileId = '%s' and t_flag = 0;", rq->fileId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, Selectlist)) {
		cout << __func__ << "SelectMySql error" << endl;
		return;
	}
	
	//保存文件信息
	STRU_FILE_INFO* info = new STRU_FILE_INFO;

	if (Selectlist.size() == 1) {
		info->filePos = atoll(Selectlist.front().c_str());
	}
	else {
		info->filePos = 0;
	}
	info->pFile = pFile;
	strcpy(info->fileId, rq->fileId);
	strcpy(info->fileName, rq->fileName);
	strcpy(info->filePATH, path);
	
	info->fileSize = rq->fileSize;
	info->userId = rq->userId;
	info->friendId = rq->friendId;

	//将文件信息保存至数据库
	if (Selectlist.empty()) {
		sprintf(sqlBuf, "insert into t_file (fileId, size, pos, sendId, recvId, path, name) value ('%s',%lld,%lld,%d,%d,'%s','%s');",
			info->fileId, info->fileSize, info->filePos, info->userId, info->friendId, info->filePATH, info->fileName);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << __func__ << " UpdateMySql error" << endl;
		}
	}
	
	m_lock.writeLock();
	m_FileIdToInfo[rq->fileId] = info;
	m_lock.writeUnlock();

	//发送文件请求回复
	strcpy(rs.fileId, rq->fileId);
	rs.result = sendfile_success;

	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
}
//处理好友发送的数据块
void CKernel::dealFileBlockRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_FILE_BLOCK_RQ* rq = (STRU_TCP_FILE_BLOCK_RQ*)buf;
	m_lock.readLock();
	//如果文件已经被取消
	auto ite =  m_FileIdToInfo.find(rq->fileId);
	if (ite == m_FileIdToInfo.end()) {
		m_lock.readUnlock();
		return;
	}
	STRU_FILE_INFO* info = ite->second;
	m_lock.readUnlock();
	

	m_lock.writeLock();
	unsigned long long pos = 0;
	if (rq->filePos == 0 && info->filePos > 0) {
		//客户端第一次发送数据
		pos = info->filePos;

		//不读取数据，直接返回指针位置
		//向客户端发送当前已经保存的数据
		STRU_TCP_FILE_BLOCK_RS rs;
		strcpy(rs.fileId, rq->fileId);

		rs.filePos = info->filePos;
		rs.friendId = info->friendId;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		m_lock.writeUnlock();
		return;
	}
	else {
		pos = min(info->filePos, rq->filePos);
	}
	//暂停接收
	if (info->status == sendfile_pause) {
		m_lock.writeUnlock();
		return;
	}
	_fseeki64(info->pFile, pos, SEEK_SET);
	long long  writeNum = fwrite(rq->block, sizeof(char), rq->blockSize, info->pFile);
	info->filePos = pos + writeNum;
	m_lock.writeUnlock();
	
	//向客户端发送当前已经保存的数据
	STRU_TCP_FILE_BLOCK_RS rs;
	strcpy(rs.fileId, rq->fileId);

	m_lock.readLock();
	rs.filePos = info->filePos;
	rs.friendId = info->friendId;
	m_lock.readUnlock();

	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

	m_lock.writeLock();
	if (info->filePos >= info->fileSize) {
		//如果好友在线向好头发送文件请求 否则关闭文件 回收map节点，
		char sqlBuf[1024] = "";
		sprintf(sqlBuf, "update t_file set pos = 0,t_flag = 1 where fileId = '%s';", rq->fileId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << __func__ << " UpdateMySql error" << endl;
		}
		TcpServer::m_threadLock.readLock();
		if (m_mapIdToSocket.find(info->friendId) == m_mapIdToSocket.end()) {
			//关闭文件 回收map节点，
			fclose(info->pFile);
			auto ite = m_FileIdToInfo.find(rq->fileId);

			delete ite->second;
			ite->second = nullptr;
			m_FileIdToInfo.erase(ite);
		}
		else {
			//发送文件请求
			STRU_TCP_SEND_FILE_RQ rq;
			strcpy(rq.fileId, info->fileId);
			strcpy(rq.fileName, info->fileName);
			rq.fileSize = info->fileSize;
			rq.friendId = info->friendId;
			rq.userId = info->userId;

			m_pMediator->SendData((char*)&rq, sizeof(rq), m_mapIdToSocket[info->friendId]);
		}
		TcpServer::m_threadLock.readUnlock();
	}
	m_lock.writeUnlock();
}
//处理发送文件回复
void CKernel::dealFriendSendFileRs(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_SEND_FILE_RS* rs = (STRU_TCP_SEND_FILE_RS*)buf;
	//发送失败 直接返回
	if (rs->result == sendfile_failed) return;

	
	//读取文件进行发送
	m_lock.readLock();
	STRU_FILE_INFO* info = m_FileIdToInfo[rs->fileId];
	STRU_TCP_FILE_BLOCK_RQ rq;
	strcpy(rq.fileId, rs->fileId);
	rq.filePos = info->filePos;
	_fseeki64(info->pFile, info->filePos, SEEK_SET);
	rq.blockSize = fread(rq.block, sizeof(char), DEF_FILE_BLOCK_SIZE, info->pFile);
	m_lock.readUnlock();

	m_pMediator->SendData((char*)&rq, sizeof(rq), lSend);
	
}
//处理文件块回复
void CKernel::dealFileBlockRs(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_FILE_BLOCK_RS* rs = (STRU_TCP_FILE_BLOCK_RS*)buf;

	m_lock.writeLock();
	//保存下载进度信息
	STRU_FILE_INFO* info = m_FileIdToInfo[rs->fileId];
	
	if (rs->filePos >= info->fileSize) {
		//传输完毕，关闭文件，删除文件，删除数据库中文件记录，回收节点
		fclose(info->pFile);

		unlink(info->filePATH);

		char sqlBuf[1024] = "";
		sprintf(sqlBuf, "delete from t_file where fileId = '%s';", rs->fileId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error" << endl;
			m_lock.writeUnlock();
			return;
		}

		delete info;
		info = nullptr;
		m_FileIdToInfo[rs->fileId] = nullptr;
		m_FileIdToInfo.erase(rs->fileId);
		m_lock.writeUnlock();
		return;
	}
	info->filePos = rs->filePos;
	_fseeki64(info->pFile, info->filePos, SEEK_SET);
	m_lock.writeUnlock();

	//发送数据
	STRU_TCP_FILE_BLOCK_RQ rq;
	rq.filePos = rs->filePos;
	strcpy(rq.fileId, rs->fileId);
	m_lock.readLock();
	rq.blockSize = fread(rq.block, sizeof(char), DEF_FILE_BLOCK_SIZE, info->pFile);
	m_lock.readUnlock();

	m_pMediator->SendData((char*)&rq, sizeof(rq), lSend);

}

//处理修改文件状态
void CKernel::dealFileStatusRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_FILE_PAUSE_RQ* rq = (STRU_TCP_FILE_PAUSE_RQ*)buf;
	m_lock.writeLock();
	m_FileIdToInfo[rq->fileId]->status = rq->flag;
	m_lock.writeUnlock();

	if (rq->kind == filekind_send) {
		if (rq->flag == sendfile_continue) {
			//继续发送
			m_lock.readLock();
			//如果文件已经被取消
			auto ite = m_FileIdToInfo.find(rq->fileId);
			if (ite == m_FileIdToInfo.end()) {
				m_lock.readUnlock();
				return;
			}
			STRU_FILE_INFO* info = ite->second;
			m_lock.readUnlock();

			//向客户端发送当前已经保存的数据
			STRU_TCP_FILE_BLOCK_RS rs;
			strcpy(rs.fileId, rq->fileId);

			m_lock.readLock();
			rs.filePos = info->filePos;
			rs.friendId = info->friendId;
			m_lock.readUnlock();

			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		}
	}
	else {
		//继续发送数据
		STRU_TCP_FILE_BLOCK_RQ fileRq;
		m_lock.readLock();
		STRU_FILE_INFO* info = m_FileIdToInfo[rq->fileId];
		strcpy(fileRq.fileId, info->fileId);
		
		fileRq.filePos = info->filePos;
		_fseeki64(info->pFile, fileRq.filePos, SEEK_SET);
		fread(fileRq.block, sizeof(char), DEF_FILE_BLOCK_SIZE, info->pFile);
		m_lock.readUnlock();

		m_pMediator->SendData((char*)&fileRq, sizeof(fileRq), lSend);
	}

}
//处理取消发送/下载文件
void CKernel::dealFileCancelRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//拆包
	STRU_TCP_FILE_CANCEL_RQ* rq = (STRU_TCP_FILE_CANCEL_RQ*)buf;
	//将文件删除，回收节点，从数据库中删除该条数据
	m_lock.writeLock();

	auto ite = m_FileIdToInfo.find(rq->fileId);
	if (ite == m_FileIdToInfo.end()) {
		STRU_TCP_FILE_CANCEL_RS rs;
		strcpy(rs.fileId, rq->fileId);
		rs.result = cancel_failed;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		m_lock.writeUnlock();
		return;
	}
	STRU_FILE_INFO* info = ite->second;
	//关闭文件
	fclose(info->pFile);
	//删除文件
	unlink(info->filePATH);
	//回收节点
	delete info;
	info = nullptr;
	ite->second = nullptr;
	m_FileIdToInfo.erase(ite);
	m_lock.writeUnlock();

	//从数据库删除该文件
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "delete from t_file where fileId = '%s'", rq->fileId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << " UpdateMySql error" << endl;
		return;
	}

	STRU_TCP_FILE_CANCEL_RS rs;
	strcpy(rs.fileId, rq->fileId);
	rs.result = cancel_success;

	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
}

//获取所有好友信息（包括自己）
void CKernel::GetAllFriendInfo(int userId) {
	//获取本用户的信息
	STRU_TCP_FRIEND_INFO userInfo;
	GetUserInfo(&userInfo, userId);

	//将用户信息通过中介者类发送给客户端
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(userId) == m_mapIdToSocket.end()) {
		cout << "CKernel::getUserList获取socket失败，userId: " << userId << endl;
		return;
	}
	m_pMediator->SendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[userId]);
	TcpServer::m_threadLock.readUnlock();
	//通过用户id查询好友列表
  	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select idB from mychat.t_friend where idA = %d;", userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	//获取好友列表信息
	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		//获取好友id
		int friendId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		//获取好友信息
		STRU_TCP_FRIEND_INFO friendInfo;
		GetUserInfo(&friendInfo, friendId);

		if (friendInfo.status == status_online) {
			//好友在线，将本人的好友信息发给他
			m_pMediator->SendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[friendId]);
		}
		//将好友的信息发送给该用户
		m_pMediator->SendData((char*)&friendInfo, sizeof(friendInfo), m_mapIdToSocket[userId]);
	}
	TcpServer::m_threadLock.readUnlock();
}
//获取用户信息
void CKernel::GetUserInfo(STRU_TCP_FRIEND_INFO* info, int userId) {
	//好友id
	info->friendId = userId;
	//查看是否在线
	TcpServer::m_threadLock.readLock();
	
	if (m_mapIdToSocket.find(userId) == m_mapIdToSocket.end()) {
		info->status = status_offline;
	}
	else {
		info->status = status_online;
	}
	TcpServer::m_threadLock.readUnlock();
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name, feeling, icon from mychat.t_user where id = %d;", userId);
	if (!m_mysql->SelectMySql(sqlBuf,3, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	//查出数据正好为3列
	if (lstSelect.size() == 3) {
		strcpy(info->name, lstSelect.front().c_str());
		lstSelect.pop_front();

		strcpy(info->feeling, lstSelect.front().c_str());
		lstSelect.pop_front();

		info->iconId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
	}

}
//获取所有群聊信息
void CKernel::GetAllGroupInfo(int userId) {
	//通过用户id获取他所在的所有群聊
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select groupId from mychat.t_groupmember where memberId = %d;", userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	while (!lstSelect.empty()) {
		STRU_TCP_GROUOP_INFO info;
		int groupId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		GetGroupInfo(&info, groupId);
		//发送给该用户
		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();

		//获取群成员信息
		GetAllGroupMemberInfo(groupId, userId);
	}
}
//获取群聊的信息
void CKernel::GetGroupInfo(STRU_TCP_GROUOP_INFO* info, int groupId) {
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name, peonum, icon, userId from mychat.t_groupchat where id = %d;", groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 4, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	info->groupId = groupId;

	//查出数据正好为4列
	if (lstSelect.size() == 4) {
		strcpy(info->name, lstSelect.front().c_str());
		lstSelect.pop_front();

		info->total = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		info->groupIconId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		info->userId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
	}
}
//获取所有某个群的群成员信息
void CKernel::GetAllGroupMemberInfo(int groupId, int userId) {
	//在成员表中查询该群所有用户的id
	//先查询自己的信息
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select memberId from mychat.t_groupmember where groupid = %d and memberId = %d;", groupId, userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	STRU_TCP_GROUOP_MEMBER_INFO Userinfo;
	if (lstSelect.size() == 1) {
		//获取到本人id，直接将信息发送给客户端
		Userinfo.groupId = groupId;
		GetGroupMemberInfo(&Userinfo, userId);
		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&Userinfo, sizeof(Userinfo), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();
	}
	lstSelect.clear();

	//再查询其他成员的信息
	sprintf_s(sqlBuf, "select memberId from mychat.t_groupmember where groupid = %d and memberId != %d;", groupId, userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	while (!lstSelect.empty()) {
		int friendId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		STRU_TCP_GROUOP_MEMBER_INFO info;
		info.groupId = groupId;
		
		GetGroupMemberInfo(&info, friendId);
		if (info.status == status_online) {
			//该好友在线，将本人的成员信息发送给好友
			TcpServer::m_threadLock.readLock();
			m_pMediator->SendData((char*)&Userinfo, sizeof(Userinfo), m_mapIdToSocket[friendId]);
			TcpServer::m_threadLock.readUnlock();
		}
		//将其他成员信息发送回客户端
		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();
	}
}
//获取某个群成员信息
void CKernel::GetGroupMemberInfo(STRU_TCP_GROUOP_MEMBER_INFO* info, int userId) {
	info->userId = userId;
	
	//查看是否在线
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(userId) == m_mapIdToSocket.end()) {
		info->status = status_offline;
	}
	else {
		info->status = status_online;
	}
	TcpServer::m_threadLock.readUnlock();

	//查看成员的名字和头像
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name, icon from mychat.t_user where id = %d;", userId);
	if (!m_mysql->SelectMySql(sqlBuf, 2, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	//查出数据正好为2列
	if (lstSelect.size() == 2) {
		strcpy(info->userName, lstSelect.front().c_str());
		lstSelect.pop_front();

		info->userIconId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
	}
}

//绑定协议映射表
void CKernel::bindProtocolMap() {
	NetProtocolMap(_DEF_TCP_REGISTER_RQ) = &CKernel::dealRegisterRq;
	NetProtocolMap(_DEF_TCP_LOGIN_RQ) = &CKernel::dealLoginRq;
	NetProtocolMap(_DEF_TCP_CHAT_RQ) = &CKernel::dealChatRq;
	NetProtocolMap(_DEF_TCP_OFFLINE_RQ) = &CKernel::dealOfflineRq;
	NetProtocolMap(_DEF_TCP_ADDFRIEND_RQ) = &CKernel::dealAddFriendRq;
	NetProtocolMap(_DEF_TCP_ADDFRIEND_RS) = &CKernel::dealAddFriendRs;
	NetProtocolMap(_DEF_TCP_DELETEFRIEND_RQ) = &CKernel::dealDeleteFriendRq;
	NetProtocolMap(_DEF_TCP_UPDATA_RQ) = &CKernel::dealUserUpdataRq;


	NetProtocolMap(_DEF_TCP_CREATEGROUP_RQ) = &CKernel::dealCreateGroupRq;
	NetProtocolMap(_DEF_TCP_JOINGROUP_RQ) = &CKernel::dealJoinGroupRq;
	NetProtocolMap(_DEF_TCP_GROUP_INFO) = &CKernel::dealGroupUpDataInfoRq;
	NetProtocolMap(_DEF_TCP_MEMBER_DELETE_RQ) = &CKernel::dealGroupDeleteRq;
	NetProtocolMap(_DEF_TCP_MEMBER_EXIT_RQ) = &CKernel::dealGroupExitRq;
	NetProtocolMap(_DEF_TCP_GROUP_CHAT_RQ) = &CKernel::dealGroupMsg;


	NetProtocolMap(_DEF_TCP_SENDFILE_RQ) = &CKernel::dealFriendSendFileRq;
	NetProtocolMap(_DEF_TCP_SENDFILE_RS) = &CKernel::dealFriendSendFileRs;
	NetProtocolMap(_DEF_TCP_FILE_BLOCK_RQ) = &CKernel::dealFileBlockRq;
	NetProtocolMap(_DEF_TCP_FILE_BLOCK_RS) = &CKernel::dealFileBlockRs;

	NetProtocolMap(_DEF_TCP_FILE_PAUSE_RQ) = &CKernel::dealFileStatusRq;
	NetProtocolMap(_DEF_TCP_FILE_CANCEL_RQ) = &CKernel::dealFileCancelRq;
	
}
//获取kernel
CKernel* CKernel::GetKernel() {
	return &m_kernel;
}

//获取随机盐
string CKernel::GetSalt(int len) {
	//设定当前线程的随机数种子
	std::random_device rd;		//随机数种子生成器
	std::mt19937 rng(rd());		//mt19937: 梅森旋转算法
	std::uniform_int_distribution<int> distribution(0, 25);	//它定义了生成指定范围内均匀分布的随机整数的功能
	string res;
	res.resize(len);
	for (int i = 0; i < len; i++) {
		res[i] = 'a' + distribution(rng);
	}
	return res;
}
//创建指定大小文件
FILE* CKernel::CreateLargeFile(char* filePath, unsigned long long fileSize)
{
	//判断该文件是否存在
	FILE* file = fopen(filePath, "r");
	if (file == nullptr) {
		//不存在 以写方式打开
		file = fopen(filePath, "wb");
		if (file == NULL)
			return nullptr;

		// 将文件指针定位到指定大小处
		int result = _fseeki64(file, fileSize - 1, SEEK_SET);
		if (result != 0)
		{
			fclose(file);
			return nullptr;
		}
		// 写入一个字节以扩展文件大小
		char byte = 0;
		result = fwrite(&byte, sizeof(char), 1, file);
		if (result != 1)
		{
			fclose(file);
			return nullptr;
		}
	}
	//存在 使用读写模式打开
	fclose(file);
	file = fopen(filePath, "r+b");
	
	return file;
}

//获取缓存在服务器中需要接收的文件
void CKernel::GetAllFile(int userId) {
	char sqlBuf[2048] = "";
	list<string> Selectlist;
	sprintf(sqlBuf, "select fileId, size, pos, sendId, path, name from t_file where recvId = %d and t_flag = 1", userId);
	if (!m_mysql->SelectMySql(sqlBuf, 6, Selectlist)) {
		cout << __func__ << " UpdateMySql error" << endl;
	}

	while ((!Selectlist.empty()) && Selectlist.size() % 6 == 0) {
		STRU_FILE_INFO* info = new STRU_FILE_INFO;
		info->friendId = userId;

		strcpy(info->fileId, Selectlist.front().c_str());
		Selectlist.pop_front();

		info->fileSize = atoll(Selectlist.front().c_str());
		Selectlist.pop_front();

		info->filePos = atoll(Selectlist.front().c_str());
		Selectlist.pop_front();

		info->userId = atoll(Selectlist.front().c_str());
		Selectlist.pop_front();

		strcpy(info->filePATH, Selectlist.front().c_str());
		Selectlist.pop_front();

		strcpy(info->fileName, Selectlist.front().c_str());
		Selectlist.pop_front();

		//打开文件
		info->pFile = fopen(info->filePATH, "r+b");
		//存入map
		m_lock.writeLock();
		m_FileIdToInfo[info->fileId] = info;
		m_lock.writeUnlock();

		//向目标客户端发送 发送文件请求
		STRU_TCP_SEND_FILE_RQ rq;
		strcpy(rq.fileId, info->fileId);
		strcpy(rq.fileName, info->fileName);
		rq.fileSize = info->fileSize;
		rq.friendId = userId;
		rq.userId = info->userId;

		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&rq, sizeof(rq), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();
	}
}