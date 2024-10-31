#pragma once
#include <string>
#include <WinSock2.h>

#define BUFSIZE 4096

class CPacket
{
public:
	CPacket() : head_(0), len_(0), cmd_(0), sum_(0) {}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
	{
		head_ = 0xFEFF;
		len_ = nSize + 4; // cmd + sum
		cmd_ = nCmd;
		memcpy((void*)data_.data(), pData, nSize);
		sum_ = 0;
		for (size_t j = 0; j < data_.size(); j++)
		{
			sum_ += BYTE(data_[j]) & 0xFF; // ȡһ���ֽ�
		}
	}
	CPacket(const CPacket& other)
	{
		head_ = other.head_;
		len_ = other.len_;
		cmd_ = other.cmd_;
		sum_ = other.sum_;
		data_ = other.data_;
	}
	CPacket(const BYTE* pData, size_t& size)
	{
		/* size��Ϊ0��������ʧ��  */
		size_t i = 0;
		for (; i < size; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFF) // �ҵ���ͷ
			{
				head_ = *(WORD*)(pData + i);
				i += 2; // ����ͷ����ͷ���������ͷ, ��ֹֻ�а�ͷû�����ݵ��������. �����ͷ����û�����ݱ��ν����ͽ���
				break;
			}
		}
		if (i + 4 + 2 + 2 >= size) // 4->len_ , 2->cmd_, 2->sum_. ����ֻ�з�������, ��û�����ݵ����
		{
			size = 0;
			return;
		}

		len_ = *(DWORD*)(pData + i); i += 4;
		if (len_ + i > size) // ������δ��ȫ���յ�
		{
			size = 0;
			return;
		}
		cmd_ = *(DWORD*)(pData + i); i += 2;
		if (len_ > 4)
		{
			data_.resize(len_ - 2 - 2); // ��cmd��ʼ��sum����
			memcpy((void*)data_.data(), pData, len_ - 4);
			i += len_ - 4;
		}
		sum_ = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < data_.size(); j++)
		{
			sum += BYTE(data_[j]) & 0xFF; // ȡһ���ֽ�
		}
		if (sum == sum_)
		{
			size = i;
			return;
		}
		size = 0;
	}
	~CPacket() = default;
	CPacket& operator=(const CPacket& other)
	{
		if (this == &other)
			return *this;
		head_ = other.head_;
		len_ = other.len_;
		cmd_ = other.cmd_;
		sum_ = other.sum_;
		data_ = other.data_;
	}

public:
	WORD head_; // �̶�λ 0xFEFF
	DWORD len_; // ����(�ӿ������ʼ����У�����)
	WORD cmd_; // ����
	std::string data_;
	WORD sum_; //��У��
};

class CServerSocket
{
public:
	static CServerSocket* getInstace();

	bool AcceptClient();
	int DealCommand(); // recv
	bool Send(const char* pData, int size);

	CServerSocket& operator=(const CServerSocket& other) = delete;

private:
	CServerSocket();
	~CServerSocket();
	bool Init();
	static void DeInit();


private:
	class CHelper
	{
	public:
		CHelper() {
			CServerSocket::getInstace();
		}
		~CHelper() {
			CServerSocket::DeInit();
		}
	};
	static CHelper helper_;

private:
	static CServerSocket* serversock_;
	SOCKET listenfd_;
	SOCKET clinetfd_;
	sockaddr_in saddr_;
	CPacket pkt_;
};

