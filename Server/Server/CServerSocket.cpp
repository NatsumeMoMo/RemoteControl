#include "pch.h" // 确保在最顶端
#include <iostream>
#include "CServerSocket.h"

CServerSocket* CServerSocket::serversock_ = nullptr;
CServerSocket::CHelper CServerSocket::helper_; // 调用CServerSocket的构造函数


CServerSocket* CServerSocket::getInstace()
{
	if (!serversock_)
	{
		serversock_ = new CServerSocket();
	}
	return serversock_;
}

CServerSocket::CServerSocket()
{
	clinetfd_ = -1;
	if (!Init())
	{
		std::cerr << "Init Socket ERROR!" << std::endl;
		exit(0);
	}
}

CServerSocket::~CServerSocket()
{
	WSACleanup();
}


bool CServerSocket::Init()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return false;
	listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd_ < 0)
		return false;

	memset(&saddr_, 0, sizeof(saddr_));
	saddr_.sin_family = AF_INET;
	saddr_.sin_addr.S_un.S_addr = INADDR_ANY; // 监听所有IP
	saddr_.sin_port = htons(9527);

	if (bind(listenfd_, (sockaddr*)&saddr_, sizeof(saddr_)) < 0)
		return false;
	listen(listenfd_, 128);

	return true;
}

void CServerSocket::DeInit()
{
	if (serversock_ != nullptr) {
		CServerSocket* tmp = serversock_;
		serversock_ = nullptr;
		delete tmp;
	}
}


bool CServerSocket::AcceptClient() {
	sockaddr_in caddr;
	int addrlen = sizeof(caddr);
	clinetfd_ = accept(listenfd_, (sockaddr*)&saddr_, &addrlen);
	if (clinetfd_ < 0)
		return false;
	return true;
}

int CServerSocket::DealCommand()
{
	if (clinetfd_ == -1) return -1;
	char buffer[BUFSIZE];
	memset(buffer, 0, sizeof(buffer));
	size_t index = 0;
	size_t pktlen = 0;
	while (true)
	{
		size_t recvlen = recv(clinetfd_, buffer + index, BUFSIZE - index, 0);
		if (recvlen <= 0)
		{
			return -1;
		}
		index += recvlen;
		pktlen = index;
		pkt_ = CPacket((const BYTE*)buffer, pktlen);
		if (recvlen > 0)
		{
			// 把后面未解析的数据挪到buffer的开头
			memmove(buffer, buffer + pktlen, BUFSIZE - pktlen);
			index -= pktlen; // 这里的recvlen和前面的recvlen值可能不同. 比如接收了2000, 但只解析了1000
			return pkt_.cmd_;
		}
	}
	return -1;
}

bool CServerSocket::Send(const char* pData, int size)
{
	if (clinetfd_ == -1)
		return false;

	return send(clinetfd_, pData, size, 0) > 0;
}
