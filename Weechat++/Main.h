
// Main.h : main header file for the Weechat++ application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "settings.h"


// CWeechatApp:
// See Main.cpp for the implementation of this class
//

class CWeechatApp : public CWinAppEx
{
public:
	CWeechatApp();

// Attributes
public:
    Settings settings;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
    virtual BOOL OnIdle(LONG lCount);

// Implementation
    BOOL  m_bHiColorIcons;

    virtual void PreLoadState();
    virtual void LoadCustomState();
    virtual void SaveCustomState();
	
    afx_msg void OnAppAbout();

	DECLARE_MESSAGE_MAP()
};

extern CWeechatApp theApp;
