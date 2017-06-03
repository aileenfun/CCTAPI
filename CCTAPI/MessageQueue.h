#pragma once
#include "Mutex.h"
#include <queue>
extern struct DFrameStruct;
#include <memory>
typedef struct UD_MESSAGE_S				//�û��Զ�����Ϣ
{
	int iType;							//��Ϣ����
	int iDataSize;						//��Ϣ�����ֽ���
	//char* pData;						//��Ϣ���ݵ�ַ
	std::unique_ptr<DFrameStruct> pAny;
	UD_MESSAGE_S()
	{
		//memset(this,0,sizeof(*this));
		//pAny.reset(new DFrameStruct);
	}

}UD_MESSAGE,*PUD_MESSAGE;

class CMessageQueue
{
public:
	CMessageQueue(void);
	~CMessageQueue(void);

public:
	int Open(int iQCap,int iDataCap);	//����:��������;��Ϣ��������
	int Close();
	//int PutMessage(UD_MESSAGE* pMsg);	//�����Ϣ
	//int GetMessage(UD_MESSAGE* pMsg);	//��ȡ��Ϣ
	int PutMessage(std::unique_ptr<UD_MESSAGE>& pMsg);
	std::unique_ptr<UD_MESSAGE> GetMessage(std::unique_ptr<UD_MESSAGE>& pMsg);
	int Reset();						//���ö���
	int GetCount();						//������Ϣ����

private:
	void ApplyResource();				//��Դ����
	void ReleaseResource();				//��Դ�ͷ�

private:
	int			m_iQueueCapacity;		//��������
	int         m_iMsgCount;			//��������Ϣ����
	int         m_iPutIndex;			//�����Ϣ����
	int         m_iGetIndex;			//��ȡ��Ϣ����
	int         m_iMsgDataCapacity;		//��Ϣ��������
	UD_MESSAGE* m_pUdMsg;				//��Ϣ������
	std::unique_ptr<UD_MESSAGE> m_u_pUdMsg;
	C_Mutex     m_Mutex;
	std::queue <std::unique_ptr<UD_MESSAGE>>queue_;
};

