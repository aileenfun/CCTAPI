#include "StdAfx.h"

#include "DataProcess.h"


#define MSG_CMD_DESTORY		0
#define MSG_CMD_SINGLE		1
#define MSG_CMD_RUN			2
#define MSG_CMD_STOP		3

#define MSG_CMD_CHANGDisplayDataBytes	7
#define MSG_CMD_CHANGEAMPLIFIER		8
#define MSG_CMD_GETAVGCOUNT  9
#define MSG_DATA_HANDALE	10

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDataProcess::CDataProcess()
{
	m_hThread=NULL;
	m_pPutMsg=NULL;
	m_pGetMsg=NULL;
	
	m_hDC=0;
	/*
	memset(&m_BitmapInfo,0,sizeof(BITMAPINFO));
	m_BitmapInfo.bmiHeader.biBitCount=24;
	m_BitmapInfo.bmiHeader.biClrImportant=0;
	m_BitmapInfo.bmiHeader.biClrUsed=0;
	m_BitmapInfo.bmiHeader.biCompression=0;

	m_BitmapInfo.bmiHeader.biPlanes=1;
	m_BitmapInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);

	m_BitmapInfo.bmiHeader.biXPelsPerMeter=0;
	m_BitmapInfo.bmiHeader.biYPelsPerMeter=0;
	*/
	m_bEnd=FALSE;
	m_lFrameCount=0;

	m_ProcType=Normal_Proc;

	m_ChangeType=Normal_Change;
	m_bCreateBmp=FALSE;
	//m_temp=NULL;
}

CDataProcess::~CDataProcess(void)
{
}

int CDataProcess::Open(int height,int width,LPVOID* lpuser,LPMV_CALLBACK2 CallBackFunc)
{
	g_height=height;
	g_width=width;
	g_width_L=width;
	h_callback=CallBackFunc;
	CGuard guard(m_Mutex);
	lpcb=lpuser;
	//m_pPutMsg = new UD_MESSAGE();
	//m_pPutMsg->pData = new char[g_height*g_width_L];
	//m_pGetMsg = new UD_MESSAGE();
	//m_pGetMsg->pData = new char[g_height*g_width_L];
	//m_MsgQueue.Open(5, g_height*g_width_L);
	//m_In = new byte[g_height*g_width_L];
	//m_Out = new tagRGB[g_height*g_width];
//	memset(m_In,0,sizeof(m_In));
	//memset(m_Out,0,sizeof(m_Out));

	m_u_pGetMsg.reset(new UD_MESSAGE);
	m_u_pPutMsg.reset(new UD_MESSAGE);
	//m_pPutMsg = new UD_MESSAGE();
	//m_pPutMsg->pData = new char[g_height*g_width_L];
	//m_pPutMsg->pAny=new DFrameStruct();
	//m_pGetMsg = new UD_MESSAGE();
	//m_pGetMsg->pData = new char[g_height*g_width_L];
	m_MsgQueue.Open(3, g_height*g_width_L);
	//m_Out = new tagRGB[g_height*g_width];
	//memset(m_In,0,sizeof(m_In));
	//memset(m_Out,0,sizeof(m_Out));

	m_bEnd=FALSE;
	m_hThread = (HANDLE)_beginthreadex(NULL,0,ThreadProcess,this,0,NULL);

	return 0;
}
int CDataProcess::Close()
{
	CGuard guard(m_Mutex);
	m_bEnd=TRUE;
	//if(m_temp!=NULL)
	//delete m_temp;
	return 0;
}

int CDataProcess::Input( std::unique_ptr<DFrameStruct>& upData,int dwSizes )
{
	CGuard guard(m_Mutex);
	m_u_pPutMsg.reset(new UD_MESSAGE);
	m_pPutMsg=m_u_pPutMsg.get();
	if(m_bEnd)
		return -1;
	m_pPutMsg->iDataSize = dwSizes;
	m_pPutMsg->pAny=std::move(upData);
	m_pPutMsg->iType = MSG_DATA_HANDALE;
	++m_lFrameCount;
	return PutMessage(m_u_pPutMsg);
}

void CDataProcess::ThreadProcessFunction()
{
	int iRet=0;
	int endflag = 1;

	while (endflag!=0)
	{
		m_u_pGetMsg.reset(new UD_MESSAGE);
		if(m_bEnd)
		{
			CloseMsgQueue();
			break;
		}
		m_u_pGetMsg=std::move(m_MsgQueue.GetMessage(m_u_pGetMsg));
		m_pGetMsg=m_u_pGetMsg.get();
		if(m_u_pGetMsg==nullptr)
		{
			Sleep(1);
			continue;
		}

		switch (m_pGetMsg->iType)
		{
		case MSG_CMD_DESTORY:
			m_MsgQueue.Reset();
			endflag=0;
			break;
		case MSG_DATA_HANDALE:
			uimData=std::move(m_pGetMsg->pAny);
			imData=uimData.get();
			ProcessIMUData(imData);
			OutPutWrapper(h_callback,lpcb);
			break;
		default:
			break;
			
		}
	}
}

unsigned int __stdcall CDataProcess::ThreadProcess( void *handle )
{
	CDataProcess *pThis = (CDataProcess *)handle;
	pThis->ThreadProcessFunction();
	return NULL;
}
int CDataProcess::OutPutWrapper(LPMV_CALLBACK2 CallBackFunc,LPVOID lpUser)
{
	CallBackFunc(imData,lpcb);
	return 0;
}
int CDataProcess::bytesToIMU(byte* bufbyte,int len,VOID* output,int outsel=0)//unsigned int* uiout,float *fout,int outsel=0)
{
	//float output;
	union
	{
		float fvalue;
		short uint16;
		unsigned int time;
		byte b[4];
	}utemp;

	for(int n=0;n<4;n++)
	{
		utemp.b[n]=0;
	}

	for(int i=0;i<len;i++)
	{
		byte tb=*(bufbyte+len-i-1);
		//byte tb=*(bufbyte+i);
		utemp.b[i]=tb;
	}
	/**((byte*)(&output) + 3) = b0;
	*((byte*)(&output) + 2) = b1;
	*((byte*)(&output) + 1) = b2;
	*((byte*)(&output) + 0) = b3;*/
	if(len==2)
		//*(float*)output=utemp.fvalue;
		*(short*) output=utemp.uint16;
	if(len==4)
		*(unsigned int*)output=utemp.time;
	return len;
}
int CDataProcess::ProcessIMUData(DFrameStruct *dFrame)
{
	int i=0;
	VOID *tempNum=new VOID*;
	IMUDataStruct *m_IMU=dFrame->IMUData.get();
	byte* m_IMUBuffer=dFrame->IMUDataBuffer.get();
	for(int j=0;j<dFrame->IMUSamplesCnt;j++)
	{
		//time Stamp
		i+=bytesToIMU(m_IMUBuffer+i,4,tempNum);
		(m_IMU+j)->timeStamp=*(unsigned int*)tempNum;
		//accel
		i+=fillAxisIMU(((IMUDataStruct*)(m_IMU+j))->accelData,m_IMUBuffer+i);
		//temperature
		i+=bytesToIMU(m_IMUBuffer+i,2,tempNum);
		(m_IMU+j)->temperData=*(short*)tempNum;
		//gyro
		i+=fillAxisIMU(((IMUDataStruct*)(m_IMU+j))->gyroData,m_IMUBuffer+i);
		
	}
	delete tempNum;
	return 0;
}
int CDataProcess::fillAxisIMU(short *imu,byte* buffer)
{
	VOID* tempNUM=new VOID*;
	int k=0;
	for(int j=0;j<3;j++,k+=2)
	{
		bytesToIMU(buffer+k,2,tempNUM,0);
		*(imu+j)=*(short*)tempNUM;
	}
	delete tempNUM;
	
	return k;
}
int CDataProcess::ProcessData(byte* m_In,int w,int h)
{
	if(!m_bEnd)
	{
		m_In = new byte[h*w];
		byte* m_processed=new byte[w*h];
		byte* m_temp=new byte[w*h];
		switch(m_ProcType)
		{
		case Xmirror_Proc:
			DoXmirrorProc(m_In,m_processed,h,w);
			break;
		case Ymirror_Proc:
			DoYmirrorProc(m_In,m_processed,h,w);
			break;
		case XYmirror_Proc:
			
			DoXmirrorProc(m_In,m_temp,h,w);
			DoYmirrorProc(m_temp,m_processed,h,w);
			
			break;
		case Normal_Proc:
		default:
			DoNormal(m_In,m_processed,h,w);
			break;
		}
		delete m_processed;
		delete m_temp;
		delete m_In;
	}
	return 0;
}

int CDataProcess::PutMessage( std::unique_ptr<UD_MESSAGE> &umsg )
{

	int iRet=0;
	while(1)
	{
		if(m_bEnd)
			break;
		iRet = m_MsgQueue.PutMessage(umsg);
		if (iRet==0)
			break;
		else if (iRet==-1)
		{
			continue;
		} 
		else
		{
			iRet=-1;
			break;
		}
		Sleep(1);
	}	
	return iRet;
}

int CDataProcess::ByteToRGB( byte *pIn ,tagRGB* pOut )
{
	static tagRGB sTempRgb;
#ifdef RGB565
	static tagRGB s565Rgb;
	byte maskGH=0x7;//0000,0111,low 3 bits
	byte maskGL=0xE0;//1110,0000,high 3 bits
	byte maskB=0x1F;//0001,1111,
	int x6=255/63;//4
	int x5=255/31;//8
	for(int i=0;i<g_height;i++)
	{
		for(int j=0;j<g_width_L;j+=2)//16bit data length 
		{
			s565Rgb.R=pIn[i * g_width_L + j]>>3;//R5
			s565Rgb.G=((pIn[i * g_width_L + j]&0x7)<<3)+((pIn[i * g_width_L + j+1]&0xE0)>>5);//G6
			s565Rgb.B=pIn[i * g_width_L + j+1]&0x1F;//B5
			sTempRgb.R=s565Rgb.R*x5;
			sTempRgb.G=s565Rgb.G*x6;
			sTempRgb.B=s565Rgb.B*x5;
			RgbChangeProc(pOut[i * g_width_L + j/2],sTempRgb);
		}
	}
#else
	for(int i=0;i<g_height;i+=2)
	{
		for(int j=0;j<g_width;j+=2)
		{
			sTempRgb.R= pIn[i * g_width + j];
			sTempRgb.G= (pIn[i * g_width + j+1]>>1)+(pIn[(i+1) * g_width + j]>>1);
			sTempRgb.B= pIn[(i+1) * g_width + j+1];
			RgbChangeProc(pOut[i * g_width + j],sTempRgb);
		}
	}
	for(int i=0;i<g_height;i+=2)
	{
		for(int j=1;j<g_width-1;j+=2)
		{
			sTempRgb.R= pIn[i * g_width + j+1];
			sTempRgb.G= (pIn[i * g_width + j]>>1)+(pIn[(i+1) * g_width + j+1]>>1);
			sTempRgb.B= pIn[(i+1) * g_width + j];
			RgbChangeProc(pOut[i * g_width + j],sTempRgb);
		}
	}
	for(int i=1;i<g_height-1;i+=2)
	{
		for(int j=0;j<g_width;j+=2)
		{
			sTempRgb.R= pIn[(i+1) * g_width + j];
			sTempRgb.G= (pIn[i * g_width + j]>>1)+(pIn[(i+1) * g_width + j+1]>>1);
			sTempRgb.B= pIn[i * g_width + j+1];
			RgbChangeProc(pOut[i * g_width + j],sTempRgb);
		}
	}
	for(int i=1;i<g_height-1;i+=2)
	{
		for(int j=1;j<g_width-1;j+=2)
		{
			sTempRgb.R= pIn[(i+1) * g_width + j+1];
			sTempRgb.G= (pIn[i * g_width + j+1]>>1)+(pIn[(i+1) * g_width + j]>>1);
			sTempRgb.B= pIn[i * g_width + j];
			RgbChangeProc(pOut[i * g_width + j],sTempRgb);
		}
	}
#endif
	return 0;
}

void CDataProcess::CloseMsgQueue()
{
	if(m_pGetMsg!=NULL)
	{
		//if(m_pGetMsg->pData!=NULL)
		//{
		//	delete[] m_pGetMsg->pData;
		//	m_pGetMsg->pData=NULL;
		//}
		//delete m_pGetMsg;
		m_pGetMsg=NULL;
	}
	if(m_pPutMsg!=NULL)
	{
		//if(m_pPutMsg->pData!=NULL)
		//{
		//	delete[] m_pPutMsg->pData;
		//	m_pPutMsg->pData=NULL;
		//}
		//delete m_pPutMsg;
		m_pPutMsg=NULL;
	}
	m_MsgQueue.Close();
}

int CDataProcess::GetFrameCount( int& fCount )
{
	CGuard guard(m_Mutex);
	fCount=m_lFrameCount;
	m_lFrameCount=0;
	return 0;
}

void CDataProcess::DoYmirrorProc(byte* pIn,byte * pOut,int h,int w)
{
	volatile byte tempbuf=0;
	for(int i=0;i<h;i++)
	{
		for (int j=0;j<w;j++)
		{
			int idx=i*w+w-1-j;
			pOut[i*w+j]=pIn[idx];
		}
	}
}
//void CDataProcess::DoYmirrorProc()
//{
//	tagRGB TempRgb;
//	for(int i=0;i<g_height;++i)
//	{
//		for(int j=0;j<g_width/2;++j )
//		{
//			memcpy(&TempRgb,&m_Out[i*g_width + j],sizeof(tagRGB));
//			memcpy(&m_Out[i*g_width + j],&m_Out[i*g_width + g_width-1-j],sizeof(tagRGB));
//			memcpy(&m_Out[i*g_width + g_width-1-j],&TempRgb,sizeof(tagRGB));
//		}
//	}
//}
void CDataProcess::DoNormal(byte* pIn,byte* pOut,int h,int w)
{
	memcpy(pOut,pIn,w*h);
}
void CDataProcess::DoXmirrorProc(byte* pIn,byte *pOut,int h,int w)
{
	//byte *tempbuf=new byte[w];
	
	for(int i=0;i<h;i++)
	{
		/*memcpy(tempbuf,&pIn[i*w],w);
		memcpy(&pIn[i*w],&pIn[(h-1-i)*w],w);
		memcpy(&pIn[(h-i-1)*w],tempbuf,w);
		memset(tempbuf,0,w);
		*/
		memcpy(&pOut[i*w],&pIn[(h-1-i)*w],w);
	}
	//delete []tempbuf;
}
//void CDataProcess::DoXmirrorProc()
//{
//	tagRGB* TempRgbBuf = new tagRGB[g_width];//如果用到，考虑指针地址循环使用
//	for(int i=0;i<g_height/2;++i)
//	{
//		memcpy(TempRgbBuf,&m_Out[i*g_width],sizeof(tagRGB)*g_width);
//		memcpy(&m_Out[i*g_width],&m_Out[(g_height-1-i)*g_width],sizeof(tagRGB)*g_width);
//		memcpy(&m_Out[(g_height-1-i)*g_width],TempRgbBuf,sizeof(tagRGB)*g_width);
//		memset(TempRgbBuf,0,sizeof(TempRgbBuf));
//	}
//	delete []TempRgbBuf;
//}

int CDataProcess::SetProcType( DataProcessType type )
{
	CGuard guard(m_Mutex);
	m_ProcType=type;
	return 0;
}

int CDataProcess::SetChangeType( RgbChangeType type )
{
	CGuard guard(m_Mutex);
	m_ChangeType=type;
	return 0;
}

void CDataProcess::RgbChangeProc( tagRGB& DestRgb,const tagRGB& OrgRgb )
{
	switch(m_ChangeType)
	{
	case RB_Change:
		DestRgb.R=OrgRgb.B;
		DestRgb.G=OrgRgb.G;
		DestRgb.B=OrgRgb.R;
		break;
	case RG_Change:
		DestRgb.R=OrgRgb.G;
		DestRgb.G=OrgRgb.R;
		DestRgb.B=OrgRgb.B;
		break;
	case GB_Change:
		DestRgb.R=OrgRgb.R;
		DestRgb.G=OrgRgb.B;
		DestRgb.B=OrgRgb.G;
		break;
	case Normal_Change:
	default:
		DestRgb.R=OrgRgb.R;
		DestRgb.G=OrgRgb.G;
		DestRgb.B=OrgRgb.B;
		break;
	}
}
