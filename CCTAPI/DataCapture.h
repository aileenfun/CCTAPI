#pragma once
#include "DataProcess.h"
#include "CyUsb.h"
/*
#include "CyAPI.h"
#include "CyUsb.h"
#include <cv.hpp>
#include <opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2\gpu\gpu.hpp>
*/
using namespace std;
class CDataCapture
{
public:
	CDataCapture(void);
	virtual ~CDataCapture(void);

public:
	int Open(CDataProcess *pProcess,int height,int width);//
	int Close();
	int Input(const LPVOID lpData,const DWORD dwSize);
	int startThread();
	unsigned long getDeviceSN();
	int getDeviceID();
	int setDeviceSN();
	int setDeviceID();
private:
		BYTE          m_byData[64];
	USB_ORDER     m_sUsbOrder;
	BOOL		  m_bSendData;

	CDataProcess *m_pDataProcess;
	int			m_iCount;		//���ݼ�����
	int			m_iRowIndex;	//������
	bool        m_bFindDbFive;	//����Ƿ��ҵ�55
	byte*		m_pInData;		//�������ݻ���
	byte*		m_pOutData;		//������ݻ���
	byte* m_pOutDataLeft;
	byte* m_pOutDataRight;
	byte* m_pIMU;
	char* m_pReadBuff;
	//long ReadDataBytes;
	long lRet;
	static unsigned int __stdcall ThreadProcess( void* handle );
	int ThreadProcessFunction();
	HANDLE m_hThread;
	volatile BOOL m_bCapture;
	int g_width_L;
	int g_width;
	int g_height;
	int fillAxisIMU(float* imu,byte* buffer);
	//DFrameStruct *dFrame;
	std::unique_ptr<DFrameStruct> dFrame;
	long lastRowIdx;
};

