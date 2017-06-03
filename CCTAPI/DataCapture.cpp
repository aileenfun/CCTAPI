#include "StdAfx.h"
#include "DataCapture.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


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
	g_width_L=g_width;
	ReadDataBytes=g_height*g_width;
	m_pReadBuff=new char[ReadDataBytes];
	m_pOutData=new byte[g_height*g_width_L];
	m_pInData=new byte[(ReadDataBytes+g_width_L+3)];
	memset(m_pInData,0,(ReadDataBytes+g_width_L+3)*sizeof(byte));
	m_bCapture=TRUE;
	m_hThread = (HANDLE)_beginthreadex(NULL,0,ThreadProcess,this,0,NULL);
	return 0;
}
int CDataCapture::Close()
{
	m_bCapture=FALSE;
	Sleep(100);
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
	return 0;
}
unsigned int __stdcall CDataCapture::ThreadProcess( void* handle )
{
	CDataCapture* pThis=(CDataCapture*)handle;
	pThis->ThreadProcessFunction();
	return 0;
}
int CDataCapture::Input( const LPVOID lpData,const DWORD dwSize )
{
	int iBytes=0;
	iBytes=dwSize+m_iCount;//m_iCount��һ�ο���ʣ������
	memcpy(m_pInData+m_iCount,lpData,dwSize);
	for(int i=0;i<iBytes;++i)
	{
		if(!m_bFindDbFive)
		{
			if(m_pInData[i]==0x55)
			{
				m_bFindDbFive=TRUE;
			}
			continue;
		}
		if(0xaa==m_pInData[i])
		{
			if((i+g_width_L+2)>=iBytes)//���ʣ�µ���󼸸����ݳ���С��video_width*2+2�кŸ��������Թ�������һ�У���������һ����
			{
				m_iCount=iBytes-i;
				memcpy(m_pInData,m_pInData+i,m_iCount);
				return 0;
			}
			m_iRowIndex=m_pInData[i+1];		//�кŸ�8λ
			m_iRowIndex<<=8;				 
			m_iRowIndex+=m_pInData[i+2];	//�кŵ�8λ
			if(m_iRowIndex>g_height+2){
				//AfxMessageBox(L"�кų���");
				return 0;
				//exit(1);
			}
			memcpy(m_pOutData+m_iRowIndex*g_width_L,m_pInData+i+3,g_width_L);
			//int tmp1 = m_pInData[i+3];
			//int tmp2 = m_pInData[i+4];
			//int tmp3 = m_pInData[i+5];
			if(m_iRowIndex>=(g_height-1))
			{
				m_pDataProcess->Input(m_pOutData,g_height*g_width_L);
				//BMPHeader(g_width_L,g_height,m_pOutData);
				//memset(m_pOutData,0,sizeof(byte)*g_height*g_width_L);
				
				/*cv::Mat frame(g_height,g_width,CV_8UC1,m_pOutData);
				cv::Mat B=frame.clone();
				cv::waitKey(1);
	cv::imshow("disp",B);
	cv::waitKey(1);*/
	//memset(m_pOutData,0,sizeof(byte)*g_height*g_width_L);
			}
			i=i+g_width_L+2;
		}
		m_bFindDbFive=FALSE;//�ҵ�0x55�������¸������ǲ���0xaa����״̬λΪFALSE,Ȼ��������0x55
	}
	return 0;
}
