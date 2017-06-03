#pragma once
#include "MessageQueue.h"
#include <memory>
//extern int g_width_L;

//#define RGB565
//#define	Array2_Index(i,j)	  i*Video_Width+j
#ifdef RGB565
#define g_width_L                       g_width*2
#else

#endif
#define Sig_L							4

#define ReadDataBytes                   1024*1024*2

typedef
	VOID
	(WINAPI * LPMV_CALLBACK2)(LPVOID lpParam, LPVOID lpUser);				
enum DataProcessType
{
	Normal_Proc,Xmirror_Proc,Ymirror_Proc,XYmirror_Proc
};
//void _stdcall RawCallBack(LPVOID lpParam,LPVOID lpUser);
enum RgbChangeType//R G B三元素互换处理
{
	Normal_Change,RG_Change,RB_Change,GB_Change
};

struct IMUDataStruct
{
	unsigned int timeStamp;
	/*float temperData;
	float accelData[3];
	float gyroData[3];*/
	short temperData;
	short accelData[3];
	 short gyroData[3];
};
struct DFrameStruct
{
	unsigned int width;
	unsigned int height;
	byte IMUPresent;
	byte imgCnt;
	unsigned int expotime;
	unsigned int IMUSamplesCnt;
	unsigned int IMUSampleLength;//18
	unsigned int timestamp;
	//byte* leftData;
	//byte* rightData;
	//IMUDataStruct *IMUData;
	//byte* IMUDataBuffer;
	std::unique_ptr<byte> leftData;
	std::unique_ptr<byte>rightData;
	std::unique_ptr<IMUDataStruct>IMUData;
	std::unique_ptr<byte>IMUDataBuffer;
	DFrameStruct():IMUSampleLength(18)
	{
	}

};
struct tagRGB
{
	byte B;
	byte G;
	byte R;
	tagRGB()
	{
		memset(this,0,sizeof(*this));
	}
};

class CDataProcess
{
public:
	CDataProcess(void);
	~CDataProcess(void);

public:
	int Open(int height,int width,LPVOID* lpuser,LPMV_CALLBACK2 CallBackFunc);
	int Close();
	int	Input(std::unique_ptr<DFrameStruct> &pData,int dwSizes);
	int GetFrameCount(int& fCount);
	int SetProcType(DataProcessType type);
	int SetChangeType(RgbChangeType type);


private:
	void ThreadProcessFunction();
	static unsigned int __stdcall ThreadProcess(void *handle);

private:
	//int ByteToRGB(byte pIn[Video_Height][Video_Width_L] ,tagRGB pOut[Video_Height][Video_Width]);
	int	ProcessData(byte* m_In,int h,int w);
	int ByteToRGB(byte *pIn ,tagRGB* pOut);
	int PutMessage(std::unique_ptr<UD_MESSAGE> &umsg);
	void CloseMsgQueue();
	void DoNormal(byte* pIn,byte *pOut,int h,int w);
	void DoXmirrorProc();//X轴镜像处理
	void DoXmirrorProc(byte* pIn,byte *pOut,int h,int w);
	void DoYmirrorProc();//Y轴镜像处理
	void DoYmirrorProc(byte* pIn,byte* pOut,int h,int w);
	void RgbChangeProc(tagRGB& DestRgb,const tagRGB& OrgRgb);//RGB元素互换处理
	void CreateBmpFile();
	LPVOID *lpcb;
private:
	HANDLE m_hThread;

	UD_MESSAGE *m_pPutMsg, *m_pGetMsg;
	std::unique_ptr<UD_MESSAGE> m_u_pPutMsg;
	std::unique_ptr<UD_MESSAGE> m_u_pGetMsg;
	CMessageQueue m_MsgQueue;
	C_Mutex	m_Mutex;
	BOOL m_bEnd;
	int m_lFrameCount;//图像帧计数
	DataProcessType m_ProcType;
	RgbChangeType m_ChangeType;
	int OutPutWrapper(LPMV_CALLBACK2 CallBackFunc,LPVOID lpUser);
	LPMV_CALLBACK2 h_callback;
	int bytesToIMU(byte*bufbyte,int len,VOID* output,int outsel);
private:
	BITMAPINFO	m_BitmapInfo;
	HDC			m_hDC;
	//byte*       m_In;
	//byte* m_processed;
	//tagRGB*     m_Out;
	int g_height,g_width,g_width_L;
	BOOL        m_bCreateBmp;

	std::unique_ptr<DFrameStruct> uimData;
	DFrameStruct *imData;
	int CDataProcess::ProcessIMUData(DFrameStruct *dFrame);
	int CDataProcess::fillAxisIMU(short *imu,byte* buffer);
};
