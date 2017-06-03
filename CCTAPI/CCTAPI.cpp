// CCTAPI.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "CCTAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CDataProcess* m_pDataProcess=NULL;
CDataCapture*  m_pDataCapture=NULL;
bool b_opened=false;
bool b_closed=true;;

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CCCTAPIApp

// CCCTAPIApp construction
BEGIN_MESSAGE_MAP(CCCTAPIApp, CWinApp)
END_MESSAGE_MAP()
CCCTAPIApp::CCCTAPIApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	b_opened=FALSE;
	m_pDataProcess=NULL;
	m_pDataCapture=NULL;
	m_lBytePerSecond=0;
	b_opened=false;
}

CCCTAPIApp theApp;
BOOL CCCTAPIApp::InitInstance()
{
	CWinApp::InitInstance();

	CyUsb_Init();
	b_opened=false;
	b_closed=true;
	return TRUE;

}
CCT_API CCCTAPIApp *Create()
{
	return new CCCTAPIApp();
}
int CCT_API CCCTAPIApp::startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc,LPVOID *lpUser,int devNum)
	//int CCT_API CCCTAPIApp::startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc,int devNum)
{
	if(b_opened)
		return -1;
	b_opened=true;
	b_closed=false;
	int m_bUsbOpen=0;
	m_bUsbOpen=OpenUsb()==0?TRUE:FALSE;
	if(m_bUsbOpen==0)
		return -1;
	m_pDataProcess=new CDataProcess();
	m_pDataCapture=new CDataCapture();
	m_pDataProcess->Open(height,width,lpUser,CallBackFunc);
	m_pDataCapture->Open(m_pDataProcess,height,width);
	return 0;
}
int CCT_API CCCTAPIApp::stopCap()
{
	if(b_closed)
		return -1;
	b_opened=false;
	b_closed=true;
	b_opened=false;
	if(m_pDataCapture!=NULL)
		m_pDataCapture->Close();
	if(m_pDataProcess!=NULL)
		m_pDataProcess->Close();
	CloseUsb();
	//CyUsb_Destroy();
	return 0;
}
int CCT_API CCCTAPIApp::setMirrorType(DataProcessType mirrortype)
{
	if(!b_opened)
	{

		return -1;
	}
	m_pDataProcess->SetProcType(mirrortype);
	return 0;
}
int CCT_API CCCTAPIApp::getDeviceID()
{
	return m_pDataCapture->getDeviceID();
}
int CCT_API CCCTAPIApp::getDeviceSN()
{
	return m_pDataCapture->getDeviceSN();
}
//int CCT_API CCCTAPIApp::getUSBDeviceCnt()
//{
//	//return getDeviceNum();
//}
//int initCCTAPI()
//{
//	b_opened=false;
//	b_closed=true;
//	m_pDataProcess=NULL;
//	m_pDataCapture=NULL;
//	return 1;
//}
//int getUSBDeviceCnt()
//{
//	return getDeviceNum();
//}
//int startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc,int devNum)
//{
//	if(b_opened)
//		return -2;
//	CyUsb_Init();
//	int m_bUsbOpen=0;
//	m_bUsbOpen=OpenUsb()==0?TRUE:FALSE;
//	if(m_bUsbOpen==0)
//		return -1;
//	m_pDataProcess=new CDataProcess();
//	m_pDataCapture=new CDataCapture();
//	m_pDataProcess->Open(height,width,CallBackFunc);
//	m_pDataCapture->Open(m_pDataProcess,height,width);
//		b_opened=true;
//	return 0;
//}
//int setMirrorType(DataProcessType mirrortype)
//{
//	if(!b_opened)
//	{
//		
//		return -1;
//	}
//	m_pDataProcess->SetProcType(mirrortype);
//	return 0;
//}
//int stopCap()
//{
//	if(!b_opened)
//		return -2;
//	b_opened=false;
//	m_pDataCapture->Close();
//	m_pDataCapture=NULL;
//	m_pDataProcess->Close();
//	m_pDataProcess=NULL;
//	CloseUsb();
//
//	//CyUsb_Destroy();
//	return 0;
//}
