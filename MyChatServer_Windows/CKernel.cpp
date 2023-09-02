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
//�򿪷�����
bool CKernel::startServer() {
	//1��������
	if (!m_pMediator->OpenNet()) {
		cout << "������ʧ��" << endl;
		return false;
	}
	//2������mysql���ݿ�
	if (!m_mysql->ConnectMySql("127.0.0.1", "root", "123456", "mychat")) {
		cout << "ConnectMySql error" << endl;
		return false;
	}
	else {
		cout << "ConnectMySql success" << endl;
	}
	return true;
}
//�رշ�����
void CKernel::closeServer() {
	//�ر�����
	if (m_pMediator) {
		m_pMediator->CloseNet();
		delete m_pMediator;
		m_pMediator = nullptr;
	}
	//�����ݿ�Ͽ�����
	if (m_mysql) {
		m_mysql->DisConnect();
		delete m_mysql;
		m_mysql = nullptr;
	}

	//��սڵ�
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

//����������пͻ��˵�����
void CKernel::dealData(char* buf, int nLen, long lSend) {
	cout << "CKernel::dealData" << endl;
	// ���
	PackType type = *(PackType*)buf;
	
	if (_DEF_PROTOCOL_BASE < type && type <= _DEF_PROTOCOL_BASE + DEF_PROTOCOL_COUNT) {
		pfun fun = NetProtocolMap(type);
		(this->*fun)(buf, nLen, lSend);
	}

	//���������Դ����
	delete[] buf;
	buf = nullptr;
}

//����ע������
void CKernel::dealRegisterRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_REGISTER_RQ* rq = (STRU_TCP_REGISTER_RQ*)buf;
	//cout << "tel:" << rq->tel << " password:" << rq->password;
	//����ע��ظ�
	STRU_TCP_REGISTER_RS rs;

	//����ֻ����Ƿ�ע���
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from mychat.t_user where tel = '%s';", rq->tel);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << "SelectMySql error" << endl;
	}
	if (lstSelect.size() > 0) {
		//�Ѿ���ע���
		rs.result = tel_is_exist;
	}
	else {
		//�ֻ���û�б�ע���
		rs.result = register_success;

		//��������������ν��м���MD5��ϣֵ
		string salt = GetSalt(DEF_SALT_SIZE);
		const char* password = m_md5.md5((salt + rq->password).c_str());
		//cout << rq->tel <<"  " << rq->password << "	" << password << "	" << salt << endl;
		//��ע����Ϣд�����ݿ�
		memset(sqlBuf, 0, 1024);
		sprintf_s(sqlBuf, "insert into mychat.t_user (tel, password, salt) value('%s','%s','%s');", rq->tel, password, salt.c_str());
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error" << endl;
		}
	}

	//����ע��ظ�
	if (!m_pMediator->SendData((char*)&rs, sizeof(rs), lSend)) {
		cout << "dealRegisterRq send Rs error" << endl;
	}
}

//�����¼����
void CKernel::dealLoginRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_LOGIN_RQ* rq = (STRU_TCP_LOGIN_RQ*)buf;
	
	//������¼�ظ���
	STRU_TCP_LOGIN_RS rs;
	
	//У���˺�����
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id,password,salt from mychat.t_user where tel = '%s';", rq->tel);
	if (!m_mysql->SelectMySql(sqlBuf, 3, lstSelect)) {
		cout << "SelectMySql error" << endl;
	}
	if (lstSelect.empty()) {
		//�û�������
		rs.result = tel_error;
		//���͵�¼�ظ�
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
	}
	else {
		//�����û�id
		int userId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();

		//����Ƿ��Ѿ���¼
		TcpServer::m_threadLock.readLock();
		if (m_mapIdToSocket.find(userId) != m_mapIdToSocket.end()) {
			//�Ѿ���¼���޷��ٽ��е�¼
			rs.result = had_login;
			//���͵�¼�ظ�
			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
			return;
		}
		TcpServer::m_threadLock.readUnlock();

		//��ȡ���������û��������
		string tempPassword = lstSelect.back() + rq->password;
		lstSelect.pop_back();

		//������ӵ�MD5��ֵ
		const char* password = m_md5.md5(tempPassword.c_str());
		cout << password << "   " << lstSelect.front() << endl;

		if (0 == strcmp(password, lstSelect.front().c_str())) {
			//������ȷ
			rs.result = login_success;
			rs.userId = userId;

			//���͵�¼�ظ������û�
			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

			//�����û���sock������map
			TcpServer::m_threadLock.writeLock();
			m_mapIdToSocket[userId] = lSend;
			TcpServer::m_threadLock.writeUnlock();

			//��ȡ�����б� ��Ⱥ���б����ʵ�֣�
			GetAllFriendInfo(userId);
			GetAllGroupInfo(userId);

			//TODO:��ȡ���ѷ��͵��ļ�
			GetAllFile(userId);
		}
		else {
			//�������
			rs.result = password_error;
			//���͵�¼�ظ�
			m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		}
	}
}
//������������
void CKernel::dealChatRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_CHAT_RQ* rq = (STRU_TCP_CHAT_RQ*)buf;
	//ͨ���û�id��ѯ�Ƿ���ڸú���
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
			//���ڸú��Ѳ������ߣ�����������ת�����ú���
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[rq->userId]);
		}
		TcpServer::m_threadLock.readUnlock();
	}
}
//������������
void CKernel::dealOfflineRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_OFFLINE_RQ* rq = (STRU_TCP_OFFLINE_RQ*)buf;

	//��������Ϣת�������еĺ��� ����Ⱥ�ġ�������ܺ�����ӣ�
	//ͨ���û�id��ѯ���к���
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select idB from mychat.t_friend where idA = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	
	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		//��ȡ����id
		int friendId = atoi(lstSelect.front().c_str());
		//�鿴�ú����Ƿ�����
		if (m_mapIdToSocket.find(friendId) != m_mapIdToSocket.end()) {
			//����������ת�����ú���
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[friendId]);
		}
		lstSelect.pop_front();
	}
	lstSelect.clear();

	//�Ͽ�����û������� �رմ��߳�
	closesocket(m_mapIdToSocket[rq->userId]);
	TcpServer::m_threadLock.readUnlock();

	//TODO:��δ��������ļ���¼�����ݿ���
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

	//�����û���sock��IdToSockmap��ɾ��
	TcpServer::m_threadLock.writeLock();
	m_mapIdToSocket.erase(rq->userId);
	TcpServer::m_threadLock.writeUnlock();

	//�����̵߳�sock��IdToSockmap��ɾ��
	TcpServer::m_threadLock.Lock();
	TcpServer::m_mapThreadIdToSocket.erase(GetCurrentThreadId());
	TcpServer::m_threadLock.UnLock();
	//ͨ���û�id��ѯ����Ⱥ��
	sprintf_s(sqlBuf, "select groupId from mychat.t_groupmember where memberId = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	STRU_TCP_GROUP_MEMBER_OFFLINE_RQ offlineRq;
	offlineRq.userId = rq->userId;
	//��ÿ��Ⱥ�������û�����Ⱥ�ĵ���������
	while (!lstSelect.empty()) {
		int groupId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		offlineRq.groupId = groupId;
		//��ȡ��Ⱥ�����г�Ա �����������͸����ߵĳ�Ա
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
				//�ó�Ա���� �������������
				m_pMediator->SendData((char*)&offlineRq, sizeof(offlineRq), m_mapIdToSocket[memberId]);
			}
			TcpServer::m_threadLock.readUnlock();
		}
	}
}

//������Ӻ�������
void CKernel::dealAddFriendRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_ADDFRIEND_RQ* rq = (STRU_TCP_ADDFRIEND_RQ*)buf;
	
	//�鿴�û��Ƿ����
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id, name from mychat.t_user where tel = %s;", rq->friendTel);
	if (!m_mysql->SelectMySql(sqlBuf, 2, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		//���û�������
		STRU_TCP_ADDFRIEND_RS rs;
		rs.result = addfriend_not_exist;
		//���ͻؿͻ���
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	if (atoi(lstSelect.front().c_str()) == rq->userId) {
		//��������Լ�Ϊ����
		STRU_TCP_ADDFRIEND_RS rs;
		rs.result = addfriend_mine;
		//���ͻؿͻ���
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}

	//strcpy(rq->userName, lstSelect.back().c_str());
	//�鿴�û��Ƿ�����
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(atoi(lstSelect.front().c_str())) == m_mapIdToSocket.end()) {
		//�û�������
		STRU_TCP_ADDFRIEND_RS rs;
		rs.result = addfriend_offline;
		//���ͻؿͻ���
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	
	//�û����ߣ������������͸����û�
	m_pMediator->SendData(buf, nLen, m_mapIdToSocket[atoi(lstSelect.front().c_str())]);
	TcpServer::m_threadLock.readUnlock();
}
//������Ӻ��ѻظ�
void CKernel::dealAddFriendRs(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_ADDFRIEND_RS* rs = (STRU_TCP_ADDFRIEND_RS*)buf;
	//��ѯ����˵�����
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name from mychat.t_user where id = %d;", rs->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	strcpy(rs->userName, lstSelect.front().c_str());
	//����Ӻ��ѻظ�ת����Ҫ��ӵ��û�
	TcpServer::m_threadLock.readUnlock();
	m_pMediator->SendData(buf, nLen, m_mapIdToSocket[rs->friendId]);
	

	if (rs->result == addfriend_success) {
		//��ӳɹ������˴˵���Ϣ���͸��Է�
		STRU_TCP_FRIEND_INFO userInof;
		STRU_TCP_FRIEND_INFO friendInof;
		GetUserInfo(&userInof, rs->friendId);
		GetUserInfo(&friendInof, rs->userId);
		//��ӳɹ��������ѹ�ϵд�����ݿ⣨д�����Σ�Ϊ�˱�֤���ζ�д��ɹ�����ʧ�ܣ���Ҫʹ������
		char sqlBuf[1024] = "";
		sprintf_s(sqlBuf, "insert into t_friend (idA, idB) values (%d, %d)", rs->userId, rs->friendId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "�������ݿ�ʧ�ܣ�sql:" << sqlBuf << endl;
			return;
		}
		sprintf_s(sqlBuf, "insert into t_friend (idA, idB) values (%d, %d)", rs->friendId, rs->userId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << "�������ݿ�ʧ�ܣ�sql:" << sqlBuf << endl;
			return;
		}

		m_pMediator->SendData((char*)&userInof, sizeof(userInof), lSend);
		m_pMediator->SendData((char*)&friendInof, sizeof(friendInof), m_mapIdToSocket[rs->friendId]);
	}
	TcpServer::m_threadLock.readUnlock();
}
//����ɾ����������
void CKernel::dealDeleteFriendRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_DELETEFRIEND_RQ* rq = (STRU_TCP_DELETEFRIEND_RQ*)buf;

	STRU_TCP_DELETEFRIEND_RS rs;
	//��ѯ�Ƿ���ں��ѹ�ϵ
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

	//�����ѹ�ϵ���е����ݽ���ɾ��
	sprintf_s(sqlBuf, "delete from t_friend where idA = %d and idB = %d;", rq->userId, rq->friendId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << "ɾ�����ݿ�ʧ�ܣ�sql:" << sqlBuf << endl;
		rs.result = deletefriend_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	sprintf_s(sqlBuf, "delete from t_friend where idA = %d and idB = %d;", rq->friendId, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << "ɾ�����ݿ�ʧ�ܣ�sql:" << sqlBuf << endl;
		rs.result = deletefriend_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}

	//ɾ���ɹ�����ɾ������������Է�����ɾ�����ѻظ����͸�����
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(rq->friendId) != m_mapIdToSocket.end()) {
		m_pMediator->SendData(buf, nLen, m_mapIdToSocket[rq->friendId]);
	}
	TcpServer::m_threadLock.readUnlock();
	
	rs.result = deletefriend_success;
	rs.friendId = rq->friendId;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
}
//��������û���Ϣ����
void CKernel::dealUserUpdataRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_UPDATA_RQ* rq = (STRU_TCP_UPDATA_RQ*)buf;

	//��ѯ�Ƿ���ڸ��û�
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_user where id = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) return;
	//��������
	sprintf_s(sqlBuf, "update t_user set name = '%s',icon = %d, feeling = '%s' where id = %d;", 
		rq->userName, rq->iconId, rq->userFeeling,rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << " UpdateMySql" << endl;
		return;
	}

	//��ѯ���û��ĺ���
	lstSelect.clear();
	sprintf_s(sqlBuf, "select idB from t_friend where idA = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		//�����µ����ݷ��͸�ÿ�����ߵĺ���
		int friendId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		if (m_mapIdToSocket.find(friendId) != m_mapIdToSocket.end()) {
			m_pMediator->SendData(buf, nLen, m_mapIdToSocket[friendId]);
		}
	}
	TcpServer::m_threadLock.readUnlock();

	//�����µ����Ϸ��͸�ÿ��Ⱥ
	//ͨ���û�id��ѯ����Ⱥ��
	sprintf_s(sqlBuf, "select groupId from mychat.t_groupmember where memberId = %d;", rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	STRU_TCP_GROUOP_MEMBER_INFO info;
	GetGroupMemberInfo(&info, rq->userId);
	//��ÿ��Ⱥ�������û����͸��û����µ�����
	while (!lstSelect.empty()) {
		int groupId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		info.groupId = groupId;
		//��ȡ��Ⱥ�����г�Ա �����Ϸ��͸����ߵĳ�Ա
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
				//�ó�Ա���� �������������
				m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[memberId]);
			}
			TcpServer::m_threadLock.readUnlock();
		}
	}
}

//������Ⱥ������
void CKernel::dealCreateGroupRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_CREATEGROUP_RQ* rq = (STRU_TCP_CREATEGROUP_RQ*)buf;
	
	//׼������Ⱥ�Ļظ���
	STRU_TCP_CREATEGROUP_RS rs;

	//��Ⱥ����Ϣ�������һ�У�ʹ�ô�����ͬʱ��Ⱥ��Ա�б������һ��
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "insert into t_groupchat (name,userId) values ('%s',%d);", rq->groupName, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << endl;
		rs.result = creategroup_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	//�����ɹ� ���ʹ���Ⱥ�Ļظ����ͻ���
	rs.result = creategroup_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

	//�����û���Ⱥ�ĵ���Ϣ���ͻؿͻ��ˣ�id�����Ǹ�Ϊ�´�����Ⱥ�ģ�
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
//�������Ⱥ������
void CKernel::dealJoinGroupRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_JOINGROUP_RQ* rq = (STRU_TCP_JOINGROUP_RQ*)buf;
	
	//׼���ظ���
	STRU_TCP_JOINGROUP_RS rs;

	//����Ⱥid�����Ƿ���ڸ�Ⱥ
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_groupchat where id = %d;", rq->groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		//�����ڸ�Ⱥ
		rs.result = joingroup_not_exist;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	
	//���ͼ�Ⱥ����ظ�
	rs.result = joingroup_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

	//����Ⱥ�͸ó�Ա�Ĺ�ϵ��	ʹ�ô洢����
	sprintf_s(sqlBuf, "call joingroup(%d, %d);", rq->groupId, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << " error" << endl;
		return;
	}
	//����Ⱥ����Ϣ���͸��ÿͻ��˺�ÿ�����ߵ�Ⱥ��Ա
	if (!lstSelect.empty()) {
		STRU_TCP_GROUOP_INFO info;
		int groupId = atoi(lstSelect.front().c_str());
		
		//���ڸ���Ⱥ����Ϣ
		//����Ⱥ����Ϣ���������ߵ�Ⱥ��Ա
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

		//��ȡ�����б�
		GetAllGroupMemberInfo(groupId, rq->userId);
	}
}
//�������Ⱥ����Ϣ����
void CKernel::dealGroupUpDataInfoRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_GROUOP_INFO* info = (STRU_TCP_GROUOP_INFO*)buf;
	
	//���Ⱥ��id���û�id�Ƿ�ƥ��
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select id from t_groupchat where id = %d and userId = %d;", info->groupId, info->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	if (lstSelect.empty()) {
		//��ƥ��ֱ���˳�
		return;
	}
	//����Ⱥ����Ϣ
	sprintf_s(sqlBuf, "update t_groupchat set name = '%s', icon = %d where id = %d;", info->name, info->groupIconId, info->groupId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << " error" << endl;
		return;
	}
	lstSelect.clear();

	//����Ⱥ����Ϣ���������ߵ�Ⱥ��Ա
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
//�����ɢȺ������
void CKernel::dealGroupDeleteRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_GROUP_DELETE_RQ* rq = (STRU_TCP_GROUP_DELETE_RQ*)buf;

	//�Ȳ�ѯ���ݿ���û��Ƿ�ΪȺ��
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
	
	//�Ƚ�������ת�������ߵ�Ⱥ��Ա��ת�������ɾ����Ϣ
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
//�����˳�Ⱥ������
void CKernel::dealGroupExitRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_GROUP_DELETE_RQ* rq = (STRU_TCP_GROUP_DELETE_RQ*)buf;

	//ɾ�����ݿ���Ⱥ�ĳ�Ա���е�����
	
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "delete from t_groupmember where groupId = %d and memberId = %d;", rq->groupId, rq->userId);
	if (!m_mysql->UpdateMySql(sqlBuf)) {
		cout << __func__ << "error" << endl;
		return;
	}

	//��ȡ��Ⱥ�ĵĳ�Ա�б� �����û��˳�Ⱥ�ĵ���Ϣת�����������ߵĺ���
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
//����Ⱥ����Ϣ
void CKernel::dealGroupMsg(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_GROUP_CHAT_RQ* rq = (STRU_TCP_GROUP_CHAT_RQ*)buf;

	//��ѯ��Ⱥ��������Ա
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select memberId from t_groupmember where groupId = %d and memberId != %d;", rq->groupId, rq->userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}

	//����Ϣת�����������߳�Ա
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

//������ѷ����ļ�����
void CKernel::dealFriendSendFileRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_SEND_FILE_RQ* rq = (STRU_TCP_SEND_FILE_RQ*)buf;
	STRU_TCP_SEND_FILE_RS rs;
	rs.friendId = rq->friendId;
	//���������û��ı����ļ���
	char path[DEF_FILE_NAME_SIZE] = "";
	sprintf(path, "D:/Server/%d", rq->friendId);
	if (!CreateDirectoryA(path, NULL) && GetLastError() != 183) {
		cout << "CreateDirectoryA error " << GetLastError()<< endl;
		rs.result = sendfile_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}
	//�������ļ� 
	sprintf(path, "D:/Server/%d/%s", rq->friendId, rq->fileName);
	FILE* pFile = CreateLargeFile(path, rq->fileSize);

	if (pFile == nullptr) {
		cout << "CreateLargeFile error" << GetLastError() << endl;
		rs.result = sendfile_failed;
		m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
		return;
	}

	//��ѯ���ݿ����Ƿ��м�¼
	list<string> Selectlist;
	char sqlBuf[2048] = "";
	sprintf(sqlBuf, "select pos from t_file where fileId = '%s' and t_flag = 0;", rq->fileId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, Selectlist)) {
		cout << __func__ << "SelectMySql error" << endl;
		return;
	}
	
	//�����ļ���Ϣ
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

	//���ļ���Ϣ���������ݿ�
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

	//�����ļ�����ظ�
	strcpy(rs.fileId, rq->fileId);
	rs.result = sendfile_success;

	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);
}
//������ѷ��͵����ݿ�
void CKernel::dealFileBlockRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_FILE_BLOCK_RQ* rq = (STRU_TCP_FILE_BLOCK_RQ*)buf;
	m_lock.readLock();
	//����ļ��Ѿ���ȡ��
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
		//�ͻ��˵�һ�η�������
		pos = info->filePos;

		//����ȡ���ݣ�ֱ�ӷ���ָ��λ��
		//��ͻ��˷��͵�ǰ�Ѿ����������
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
	//��ͣ����
	if (info->status == sendfile_pause) {
		m_lock.writeUnlock();
		return;
	}
	_fseeki64(info->pFile, pos, SEEK_SET);
	long long  writeNum = fwrite(rq->block, sizeof(char), rq->blockSize, info->pFile);
	info->filePos = pos + writeNum;
	m_lock.writeUnlock();
	
	//��ͻ��˷��͵�ǰ�Ѿ����������
	STRU_TCP_FILE_BLOCK_RS rs;
	strcpy(rs.fileId, rq->fileId);

	m_lock.readLock();
	rs.filePos = info->filePos;
	rs.friendId = info->friendId;
	m_lock.readUnlock();

	m_pMediator->SendData((char*)&rs, sizeof(rs), lSend);

	m_lock.writeLock();
	if (info->filePos >= info->fileSize) {
		//��������������ͷ�����ļ����� ����ر��ļ� ����map�ڵ㣬
		char sqlBuf[1024] = "";
		sprintf(sqlBuf, "update t_file set pos = 0,t_flag = 1 where fileId = '%s';", rq->fileId);
		if (!m_mysql->UpdateMySql(sqlBuf)) {
			cout << __func__ << " UpdateMySql error" << endl;
		}
		TcpServer::m_threadLock.readLock();
		if (m_mapIdToSocket.find(info->friendId) == m_mapIdToSocket.end()) {
			//�ر��ļ� ����map�ڵ㣬
			fclose(info->pFile);
			auto ite = m_FileIdToInfo.find(rq->fileId);

			delete ite->second;
			ite->second = nullptr;
			m_FileIdToInfo.erase(ite);
		}
		else {
			//�����ļ�����
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
//�������ļ��ظ�
void CKernel::dealFriendSendFileRs(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_SEND_FILE_RS* rs = (STRU_TCP_SEND_FILE_RS*)buf;
	//����ʧ�� ֱ�ӷ���
	if (rs->result == sendfile_failed) return;

	
	//��ȡ�ļ����з���
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
//�����ļ���ظ�
void CKernel::dealFileBlockRs(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_FILE_BLOCK_RS* rs = (STRU_TCP_FILE_BLOCK_RS*)buf;

	m_lock.writeLock();
	//�������ؽ�����Ϣ
	STRU_FILE_INFO* info = m_FileIdToInfo[rs->fileId];
	
	if (rs->filePos >= info->fileSize) {
		//������ϣ��ر��ļ���ɾ���ļ���ɾ�����ݿ����ļ���¼�����սڵ�
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

	//��������
	STRU_TCP_FILE_BLOCK_RQ rq;
	rq.filePos = rs->filePos;
	strcpy(rq.fileId, rs->fileId);
	m_lock.readLock();
	rq.blockSize = fread(rq.block, sizeof(char), DEF_FILE_BLOCK_SIZE, info->pFile);
	m_lock.readUnlock();

	m_pMediator->SendData((char*)&rq, sizeof(rq), lSend);

}

//�����޸��ļ�״̬
void CKernel::dealFileStatusRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_FILE_PAUSE_RQ* rq = (STRU_TCP_FILE_PAUSE_RQ*)buf;
	m_lock.writeLock();
	m_FileIdToInfo[rq->fileId]->status = rq->flag;
	m_lock.writeUnlock();

	if (rq->kind == filekind_send) {
		if (rq->flag == sendfile_continue) {
			//��������
			m_lock.readLock();
			//����ļ��Ѿ���ȡ��
			auto ite = m_FileIdToInfo.find(rq->fileId);
			if (ite == m_FileIdToInfo.end()) {
				m_lock.readUnlock();
				return;
			}
			STRU_FILE_INFO* info = ite->second;
			m_lock.readUnlock();

			//��ͻ��˷��͵�ǰ�Ѿ����������
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
		//������������
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
//����ȡ������/�����ļ�
void CKernel::dealFileCancelRq(char* buf, int nLen, long lSend) {
	cout << __func__ << endl;
	//���
	STRU_TCP_FILE_CANCEL_RQ* rq = (STRU_TCP_FILE_CANCEL_RQ*)buf;
	//���ļ�ɾ�������սڵ㣬�����ݿ���ɾ����������
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
	//�ر��ļ�
	fclose(info->pFile);
	//ɾ���ļ�
	unlink(info->filePATH);
	//���սڵ�
	delete info;
	info = nullptr;
	ite->second = nullptr;
	m_FileIdToInfo.erase(ite);
	m_lock.writeUnlock();

	//�����ݿ�ɾ�����ļ�
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

//��ȡ���к�����Ϣ�������Լ���
void CKernel::GetAllFriendInfo(int userId) {
	//��ȡ���û�����Ϣ
	STRU_TCP_FRIEND_INFO userInfo;
	GetUserInfo(&userInfo, userId);

	//���û���Ϣͨ���н����෢�͸��ͻ���
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(userId) == m_mapIdToSocket.end()) {
		cout << "CKernel::getUserList��ȡsocketʧ�ܣ�userId: " << userId << endl;
		return;
	}
	m_pMediator->SendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[userId]);
	TcpServer::m_threadLock.readUnlock();
	//ͨ���û�id��ѯ�����б�
  	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select idB from mychat.t_friend where idA = %d;", userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	//��ȡ�����б���Ϣ
	TcpServer::m_threadLock.readLock();
	while (!lstSelect.empty()) {
		//��ȡ����id
		int friendId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
		//��ȡ������Ϣ
		STRU_TCP_FRIEND_INFO friendInfo;
		GetUserInfo(&friendInfo, friendId);

		if (friendInfo.status == status_online) {
			//�������ߣ������˵ĺ�����Ϣ������
			m_pMediator->SendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[friendId]);
		}
		//�����ѵ���Ϣ���͸����û�
		m_pMediator->SendData((char*)&friendInfo, sizeof(friendInfo), m_mapIdToSocket[userId]);
	}
	TcpServer::m_threadLock.readUnlock();
}
//��ȡ�û���Ϣ
void CKernel::GetUserInfo(STRU_TCP_FRIEND_INFO* info, int userId) {
	//����id
	info->friendId = userId;
	//�鿴�Ƿ�����
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
	//�����������Ϊ3��
	if (lstSelect.size() == 3) {
		strcpy(info->name, lstSelect.front().c_str());
		lstSelect.pop_front();

		strcpy(info->feeling, lstSelect.front().c_str());
		lstSelect.pop_front();

		info->iconId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
	}

}
//��ȡ����Ⱥ����Ϣ
void CKernel::GetAllGroupInfo(int userId) {
	//ͨ���û�id��ȡ�����ڵ�����Ⱥ��
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
		//���͸����û�
		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();

		//��ȡȺ��Ա��Ϣ
		GetAllGroupMemberInfo(groupId, userId);
	}
}
//��ȡȺ�ĵ���Ϣ
void CKernel::GetGroupInfo(STRU_TCP_GROUOP_INFO* info, int groupId) {
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name, peonum, icon, userId from mychat.t_groupchat where id = %d;", groupId);
	if (!m_mysql->SelectMySql(sqlBuf, 4, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	info->groupId = groupId;

	//�����������Ϊ4��
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
//��ȡ����ĳ��Ⱥ��Ⱥ��Ա��Ϣ
void CKernel::GetAllGroupMemberInfo(int groupId, int userId) {
	//�ڳ�Ա���в�ѯ��Ⱥ�����û���id
	//�Ȳ�ѯ�Լ�����Ϣ
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select memberId from mychat.t_groupmember where groupid = %d and memberId = %d;", groupId, userId);
	if (!m_mysql->SelectMySql(sqlBuf, 1, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	STRU_TCP_GROUOP_MEMBER_INFO Userinfo;
	if (lstSelect.size() == 1) {
		//��ȡ������id��ֱ�ӽ���Ϣ���͸��ͻ���
		Userinfo.groupId = groupId;
		GetGroupMemberInfo(&Userinfo, userId);
		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&Userinfo, sizeof(Userinfo), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();
	}
	lstSelect.clear();

	//�ٲ�ѯ������Ա����Ϣ
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
			//�ú������ߣ������˵ĳ�Ա��Ϣ���͸�����
			TcpServer::m_threadLock.readLock();
			m_pMediator->SendData((char*)&Userinfo, sizeof(Userinfo), m_mapIdToSocket[friendId]);
			TcpServer::m_threadLock.readUnlock();
		}
		//��������Ա��Ϣ���ͻؿͻ���
		TcpServer::m_threadLock.readLock();
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[userId]);
		TcpServer::m_threadLock.readUnlock();
	}
}
//��ȡĳ��Ⱥ��Ա��Ϣ
void CKernel::GetGroupMemberInfo(STRU_TCP_GROUOP_MEMBER_INFO* info, int userId) {
	info->userId = userId;
	
	//�鿴�Ƿ�����
	TcpServer::m_threadLock.readLock();
	if (m_mapIdToSocket.find(userId) == m_mapIdToSocket.end()) {
		info->status = status_offline;
	}
	else {
		info->status = status_online;
	}
	TcpServer::m_threadLock.readUnlock();

	//�鿴��Ա�����ֺ�ͷ��
	list<string> lstSelect;
	char sqlBuf[1024] = "";
	sprintf_s(sqlBuf, "select name, icon from mychat.t_user where id = %d;", userId);
	if (!m_mysql->SelectMySql(sqlBuf, 2, lstSelect)) {
		cout << __func__ << "error" << endl;
		return;
	}
	//�����������Ϊ2��
	if (lstSelect.size() == 2) {
		strcpy(info->userName, lstSelect.front().c_str());
		lstSelect.pop_front();

		info->userIconId = atoi(lstSelect.front().c_str());
		lstSelect.pop_front();
	}
}

//��Э��ӳ���
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
//��ȡkernel
CKernel* CKernel::GetKernel() {
	return &m_kernel;
}

//��ȡ�����
string CKernel::GetSalt(int len) {
	//�趨��ǰ�̵߳����������
	std::random_device rd;		//���������������
	std::mt19937 rng(rd());		//mt19937: ÷ɭ��ת�㷨
	std::uniform_int_distribution<int> distribution(0, 25);	//������������ָ����Χ�ھ��ȷֲ�����������Ĺ���
	string res;
	res.resize(len);
	for (int i = 0; i < len; i++) {
		res[i] = 'a' + distribution(rng);
	}
	return res;
}
//����ָ����С�ļ�
FILE* CKernel::CreateLargeFile(char* filePath, unsigned long long fileSize)
{
	//�жϸ��ļ��Ƿ����
	FILE* file = fopen(filePath, "r");
	if (file == nullptr) {
		//������ ��д��ʽ��
		file = fopen(filePath, "wb");
		if (file == NULL)
			return nullptr;

		// ���ļ�ָ�붨λ��ָ����С��
		int result = _fseeki64(file, fileSize - 1, SEEK_SET);
		if (result != 0)
		{
			fclose(file);
			return nullptr;
		}
		// д��һ���ֽ�����չ�ļ���С
		char byte = 0;
		result = fwrite(&byte, sizeof(char), 1, file);
		if (result != 1)
		{
			fclose(file);
			return nullptr;
		}
	}
	//���� ʹ�ö�дģʽ��
	fclose(file);
	file = fopen(filePath, "r+b");
	
	return file;
}

//��ȡ�����ڷ���������Ҫ���յ��ļ�
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

		//���ļ�
		info->pFile = fopen(info->filePATH, "r+b");
		//����map
		m_lock.writeLock();
		m_FileIdToInfo[info->fileId] = info;
		m_lock.writeUnlock();

		//��Ŀ��ͻ��˷��� �����ļ�����
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