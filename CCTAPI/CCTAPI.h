// CCTAPI.h : main header file for the CCTAPI DLL
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#ifdef COMPILE_API
#define CCT_API __declspec(dllexport) 
#else 
#define CCT_API __declspec(dllimport)
#endif

#include "resource.h"		// main symbols
#include "DataCapture.h"
#include "CyUsb.h"
#include <vector>
// CCCTAPIApp
// See CCTAPI.cpp for the implementation of this class
//
class ICCTAPI{
public:
	virtual void destory()=0;
	virtual int startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc,int devNum)=0;
	virtual int stopCap()=0;
	virtual int setMirrorType(DataProcessType mirrortype)=0;
	virtual int getUSBDeviceCnt()=0;
	virtual int getDeviceID();
	virtual int getDeviceSN();
	//virtual void  CCCTAPIApp()=0;

};

class  CCCTAPIApp :public CWinApp// //public ICCTAPI,
{
public:
	CCCTAPIApp();
	CCT_API int startCap(int height,int width,LPMV_CALLBACK2 CallBackFunc,LPVOID* lpUser,int devNum);
	CCT_API int stopCap();
	CCT_API int setMirrorType(DataProcessType mirrortype);
	CCT_API int getUSBDeviceCnt();
	CCT_API int getDeviceID();
	CCT_API int getDeviceSN();
	void destory();
	// Overrides

public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()

private:
	char*         m_pReadBuff;
	long          m_lBytePerSecond;
	bool m_bUsbOpen;
};

extern "C" __declspec(dllexport)  ICCTAPI * APIENTRY   create_CCTAPI();
