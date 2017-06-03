// CCTAPI.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "CCTAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CDataProcess* m_pDataProcess;
CDataCapture*  m_pDataCapture;
bool b_opened;
	bool b_closed;
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

BEGIN_MESSAGE_MAP(CCCTAPIApp, CWinApp)
END_MESSAGE_MAP()


// CCCTAPIApp construction

CCCTAPIApp::CCCTAPIApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_bUsbOpen=FALSE;
	m_pDataProcess=NULL;
	m_pDataCapture=NULL;
	m_lBytePerSecond=0;
	b_opened=false;
}


// The one and only CCCTAPIApp object

CCCTAPIApp theApp;
	

// CCCTAPIApp initialization

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
int CCT_API CCCTAPIApp::startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc)
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
	m_pDataProcess->Open(height,width,CallBackFunc);
	m_pDataCapture->Open(m_pDataProcess,height,width);
	return 0;
}
int CCT_API CCCTAPIApp::stopCap()
{
	if(b_closed)
		return -1;
	b_opened=false;
	b_closed=true;
	m_pDataCapture->Close();
	m_pDataProcess->Close();
	CloseUsb();
	//CyUsb_Destroy();
	return 0;
}