#include "StdAfx.h"
#include "DataCapture.h"
#include <assert.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#pragma comment(lib, "CyAPI.LIB")
CDataCapture::CDataCapture(void)
{
	m_pDataProcess=NULL;
	m_iCount=0;		
	m_iRowIndex=0;		
	m_bFindDbFive=FALSE;	
	m_pInData=NULL;		
	m_pOutData=NULL;	
	
}

CDataCapture::~CDataCapture(void)
{
	Close();
}
int CDataCapture::Open(CDataProcess *pProcess,int height,int width)// 
{
	m_pDataProcess=pProcess;
	g_height=height;
	g_width=width;
	g_width_L=g_width*2;
//	ReadDataBytes=g_height*g_width*4;
	m_pReadBuff=new char[ReadDataBytes];
	m_pOutData=new byte[g_height*g_width];
	m_pInData=new byte[(ReadDataBytes*2)];
	memset(m_pInData,0,(ReadDataBytes+g_width_L+3)*sizeof(byte));
	m_bCapture=TRUE;
	m_hThread = (HANDLE)_beginthreadex(NULL,0,ThreadProcess,this,0,NULL);
	return 0;
}
int CDataCapture::getDeviceID()
{
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.ReqCode=0xF1;
	m_sUsbOrder.Direction=1;
	SendOrder(&m_sUsbOrder);
	return m_byData[0];
}
unsigned long CDataCapture::getDeviceSN()
{
		m_sUsbOrder.DataBytes=4;
	m_sUsbOrder.ReqCode=0xF3;
	m_sUsbOrder.Direction=1;
	SendOrder(&m_sUsbOrder);
	unsigned long temp=m_byData[3]<<24+m_byData[2]<<16+m_byData[1]<<8+m_byData[0];
	return temp;
}
int CDataCapture::setDeviceID()
{
	m_byData[0]=0xAD;
	m_sUsbOrder.DataBytes=1;
	m_sUsbOrder.ReqCode=0xF1;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	return 0;
}
int CDataCapture::setDeviceSN()
{
	m_byData[0]=0x11;
	m_byData[1]=0x22;
	m_byData[2]=0x33;
	m_byData[3]=0x44;
	m_sUsbOrder.DataBytes=4;
	m_sUsbOrder.ReqCode=0xF3;
	m_sUsbOrder.Direction=0;
	SendOrder(&m_sUsbOrder);
	return 0;
}
int CDataCapture::Close()
{
	m_bCapture=FALSE;
	Sleep(500);
	if(m_pOutData!=NULL)
	{
		delete[] m_pOutData;
		m_pOutData=NULL;
	}
	if(m_pInData!=NULL)
	{
		delete[] m_pInData;
		m_pInData=NULL;
	}

	delete[] m_pReadBuff;
	
	return 0;
}
int CDataCapture::ThreadProcessFunction()
{
	long lRet=0;
	while(m_bCapture)
	{
		lRet=ReadDataBytes;
		ReadData(m_pReadBuff,lRet);
		if(lRet>0)
		{
			if(!m_bCapture) break;
			Input(m_pReadBuff,lRet);
		}
	}
	if(m_pReadBuff!=NULL)
	{
		delete []m_pReadBuff;
		m_pReadBuff=NULL;
	}
	return 0;
}
unsigned int __stdcall CDataCapture::ThreadProcess( void* handle )
{
	CDataCapture* pThis=(CDataCapture*)handle;
	pThis->ThreadProcessFunction();
	return 0;
}
void checkcolum(byte* checkbuf,int w)
{
	byte k=0;
	for(int i=0;i<w;i++)
	{
		if(checkbuf[i]!=k)
		{
			//error	
			ASSERT("ERROR");
		}
		k++;
	}
}
int CDataCapture::Input( const LPVOID lpData,const DWORD dwSize )
{

	int iBytes=0;
	iBytes=dwSize+m_iCount;//m_iCount上一次拷贝剩余数据
	memcpy(m_pInData+m_iCount,lpData,dwSize);
	bool b_header=false,b_imu=false;
	for(int i=0;i<iBytes;++i)
	{
		int imulen=0;
		if ((i + g_width * g_height * 3) >= iBytes)//如果剩下的最后几个数据长度小于video_width*2+2行号个，不足以构成完整一行，拷贝到下一缓存
			{
				m_iCount = iBytes - i;
				memcpy(m_pInData, m_pInData + i, m_iCount);
				return 0;
			}
		/*
		if(!m_bFindDbFive)
		{
			if(m_pInData[i]==0x44)
			{
				m_bFindDbFive=TRUE;
			}
			continue;
		}
		if(0xbb==m_pInData[i])
		{
			if((i+g_width*g_height*3+2)>=iBytes)//如果剩下的最后几个数据长度小于video_width*2+2行号个，不足以构成完整一行，拷贝到下一缓存
			{
				m_iCount=iBytes-i;
				memcpy(m_pInData,m_pInData+i+2,m_iCount);
				return 0;
			}
			dFrame.reset(new DFrameStruct);
			(*dFrame).width=640;
			(*dFrame).height=480;
			(*dFrame).leftData.reset(new byte[640*480]);

			long datalen=g_height*g_width;
			memcpy((*dFrame).leftData.get(),m_pInData+i+1,datalen);
			checkcolum((*dFrame).leftData.get(),640);
			m_pDataProcess->Input(dFrame,datalen);
			i=i+datalen*2-1;

		}
		m_bFindDbFive=FALSE;//找到0x55后，无论下个数据是不是0xaa都置状态位为FALSE,然后重新找0x55
		*/
		
		if(m_pInData[i]==0x33&&m_pInData[i+1]==0xcc&&m_pInData[i+14]==0x22&&m_pInData[i+15]==0xdd&&b_header==FALSE)
		{
			
			dFrame.reset(new DFrameStruct);
			(*dFrame).expotime=m_pInData[i+2]<<8+m_pInData[i+3];
			(*dFrame).imgCnt=1;//m_pInData[i+4];
			(*dFrame).IMUSamplesCnt=m_pInData[i+5];
			imulen=dFrame->IMUSamplesCnt*dFrame->IMUSampleLength;
			checkcolum(m_pInData+i+16+imulen+4,640);
			//(*dFrame).IMUPresent=m_pInData[i+5];
			unsigned int temp=m_pInData[i+6]<<8;
			temp=temp+m_pInData[i+7];
			(*dFrame).height=temp;
			temp=m_pInData[i+8]<<8;
			temp+=m_pInData[i+9];
			(*dFrame).width=temp;//(m_pInData[i+8]<<8+m_pInData[i+9]);
			assert((*dFrame).width==640);
			assert((*dFrame).height==480);
			(*dFrame).timestamp=m_pInData[i+10]<<8*3+m_pInData[i+11]<<8*2+m_pInData[i+12]<<8+m_pInData[i+13];
			dFrame->IMUData.reset(new IMUDataStruct[dFrame->IMUSamplesCnt]);
			dFrame->IMUDataBuffer.reset(new byte[imulen]);
			dFrame->leftData.reset(new byte[dFrame->height*dFrame->width]);
			dFrame->rightData.reset(new byte[dFrame->height*dFrame->width]);
			m_pOutDataLeft=dFrame->leftData.get();
			m_pOutDataRight=dFrame->rightData.get();
			m_pIMU=dFrame->IMUDataBuffer.get();
			i+=16;
			b_header=true;
		}
		if(imulen>0&&m_pInData[i]==0x66&&m_pInData[i+1]==0xdd&&m_pInData[i+imulen+2]==0x44&&m_pInData[i+imulen+3]==0xbb&&b_header)
		{
			memcpy(m_pIMU,m_pInData+i+2,imulen);
			i+=imulen+4;
			
			b_imu=true;
		}
		//else if(b_header==true&&m_bFindDbFive==false)
		//{
		//	b_header=false;
		//	b_imu=false;
		//}
		//////
		if (b_header&&(b_imu||imulen==0))
		{
			
			unsigned int datalen = dFrame->width * dFrame->height;
			memcpy(m_pOutDataLeft, m_pInData + i, datalen);
			if((*dFrame).imgCnt==2)
			{
				memcpy(m_pOutDataRight, m_pInData + i + datalen, datalen);
			}
			
			m_pDataProcess->Input(dFrame, (*dFrame).width * (*dFrame).height);
			b_imu = false;
			b_header = false;
			m_bFindDbFive = false;
			i = i + datalen-1;
		}
		
		////////
		/*
		if(m_pInData[i]==0x55&&m_pInData[i+1]==0xaa&&b_imu)
		{
			m_bFindDbFive=true;
			i=i+1;
			if((i+g_width_L+2)>=iBytes)//如果剩下的最后几个数据长度小于video_width*2+2行号个，不足以构成完整一行，拷贝到下一缓存
			{
				m_iCount=iBytes-i;
				memcpy(m_pInData,m_pInData+i,m_iCount);
				return 0;
			}
			m_iRowIndex=m_pInData[i+1];		//行号高8位
			m_iRowIndex<<=8;				 
			m_iRowIndex+=m_pInData[i+2];	//行号低8位
			if(lastRowIdx>m_iRowIndex&&m_iRowIndex<g_height-1)
			{
				int k=0;
			}
				lastRowIdx=m_iRowIndex;
				//memcpy(m_pOutData+m_iRowIndex*g_width_L,m_pInData+i+3,g_width_L);
				if(m_iRowIndex<dFrame->height)//left img data
				{
					memcpy(m_pOutDataLeft+m_iRowIndex*dFrame->width,m_pInData+i+3,dFrame->width);
				}
				else
				{
					memcpy(m_pOutDataRight+(m_iRowIndex-480)*dFrame->width,m_pInData+i+3,dFrame->width);
				}
				
				//memcpy(m_pOutDataLeft+m_iRowIndex*dFrame->width,m_pInData+i+3,dFrame->width);
				//memcpy(m_pOutDataRight+m_iRowIndex*dFrame->width,m_pInData+i+3+dFrame->width,dFrame->width);
			if(m_iRowIndex>=(g_height*2-1))
			{
				m_pDataProcess->Input(dFrame,(*dFrame).width*(*dFrame).height);
				
				b_imu=false;
				b_header=false;
				m_bFindDbFive=false;
				lastRowIdx=0;
				break;
			}
			
			i=i+g_width+2;
			
		}
	
		else if(b_imu==true)
		{
			//b_imu=false;
			b_header=false;
			m_bFindDbFive=false;
		}
		*/
		//m_bFindDbFive=FALSE;//找到0x55后，无论下个数据是不是0xaa都置状态位为FALSE,然后重新找0x55
		
	}//for
	return 0;
}
