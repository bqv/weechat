
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "BufferView.h"
#include "NicklistView.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNCREATE(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
    CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
    CMFCToolBarImages m_UserImages;
    CBufferView       m_wndBufferView;
    CNicklistView     m_wndNicklistView;
    CMFCCaptionBar    m_wndCaptionBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnWindowManager();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnViewCustomize();
    afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
    afx_msg void OnViewCaptionBar();
    afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
    afx_msg void OnOptions();
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

    BOOL CreateDockingWindows();
    void SetDockingWindowIcons(BOOL bHiColorIcons);
    BOOL CreateCaptionBar();
};


