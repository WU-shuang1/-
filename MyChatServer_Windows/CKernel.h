#pragma once
#include <iostream>
#include"TcpServerMediator.h"
#include "CMySql.h"
#include "pch.h"
#include "packDef.h"
#include <map>
using namespace std;

class CKernel;
typedef void (CKernel::*pfun) (char*, int, long);

class CKernel
{
private:
	CKernel();
	CKernel(const CKernel& other){}
	~CKernel();
public:
	//打开服务器
	bool startServer();
	//关闭服务器
	void closeServer();

	//处理接收到的所有数据
	void dealData(char* buf, int nLen, long lSend);
	//处理注册请求
	void dealRegisterRq(char* buf, int nLen, long lSend);
	//处理登录请求
	void dealLoginRq(char* buf, int nLen, long lSend);
	//处理聊天请求
	void dealChatRq(char* buf, int nLen, long lSend);
	//处理下线请求
	void dealOfflineRq(char* buf, int nLen, long lSend);
	//处理添加好友请求
	void dealAddFriendRq(char* buf, int nLen, long lSend);
	//处理添加好友回复
	void dealAddFriendRs(char* buf, int nLen, long lSend);
	//处理删除好友请求
	void dealDeleteFriendRq(char* buf, int nLen, long lSend);
	//处理更新用户信息请求
	void dealUserUpdataRq(char* buf, int nLen, long lSend);
	
	//处理创建群聊请求
	void dealCreateGroupRq(char* buf, int nLen, long lSend);
	//处理加入群聊请求
	void dealJoinGroupRq(char* buf, int nLen, long lSend);
	//处理更新群聊信息请求
	void dealGroupUpDataInfoRq(char* buf, int nLen, long lSend);
	//处理解散群聊请求
	void dealGroupDeleteRq(char* buf, int nLen, long lSend);
	//处理退出群聊请求
	void dealGroupExitRq(char* buf, int nLen, long lSend);
	//处理群聊消息
	void dealGroupMsg(char* buf, int nLen, long lSend);

	//处理好友发送文件请求
	void dealFriendSendFileRq(char* buf, int nLen, long lSend);
	//处理发送文件回复
	void dealFriendSendFileRs(char* buf, int nLen, long lSend);

	//处理好友发送的数据块
	void dealFileBlockRq(char* buf, int nLen, long lSend);
	//处理文件块回复
	void dealFileBlockRs(char* buf, int nLen, long lSend);

	//处理修改文件状态
	void dealFileStatusRq(char* buf, int nLen, long lSend);
	//处理取消发送/下载文件
	void dealFileCancelRq(char* buf, int nLen, long lSend);

	//获取所有好友信息（包括自己）
	void GetAllFriendInfo(int userId);
	//获取用户信息
	void GetUserInfo(STRU_TCP_FRIEND_INFO* info, int userId);
	//获取所有群聊信息
	void GetAllGroupInfo(int userId);
	//获取群聊的信息
	void GetGroupInfo(STRU_TCP_GROUOP_INFO* info, int groupId);
	//获取所有群成员信息
	void GetAllGroupMemberInfo(int groupId ,int userId);
	//获取某个群成员信息
	void GetGroupMemberInfo(STRU_TCP_GROUOP_MEMBER_INFO* info, int userId);

	//获取缓存在服务器中需要接收的文件
	void GetAllFile(int userId);

	//绑定协议映射表
	void bindProtocolMap();
	//获取kernel对象
	static CKernel* GetKernel();
	//获取随机盐
	string GetSalt(int len);
	//创建指定大小文件
	FILE* CreateLargeFile(char* filePath, unsigned long long fileSize);
	
public:
	CLock m_lock;		//文件id-文件信息的map的读写锁
	CMD5Encrypt m_md5;
	INetMediator* m_pMediator;
	CMySql* m_mysql;
	map <int, SOCKET> m_mapIdToSocket;		//使用map保存用户的sock，其中用户id为key，sock为value
	pfun m_netProtocolMap[DEF_PROTOCOL_COUNT];			//网络协议映射表
	map <string, STRU_FILE_INFO*> m_FileIdToInfo;	//保存文件信息
private:
	static CKernel m_kernel;
};

