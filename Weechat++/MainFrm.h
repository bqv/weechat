
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "BufferView.h"
#include "WeechatView.h"
#include "NicklistView.h"
#include "RelayConnection.h"
#include "Relay/Handler.h"
#include "Relay/IRelayClient.h"

class CMainFrame : public CMDIFrameWndEx, public IRelayClient
{
	DECLARE_DYNCREATE(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
    CMap<CString, LPCTSTR, CNicklist, const CNicklist&> m_nicklistCache;
    CRelayConnection m_relayConn;
    Handler m_relayHandler;
    int m_relayType;
    CString m_relayHost;
    UINT m_relayPort;
    CString m_relayPass;
    CString m_status;

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
    void ProtocolAssert(const bool expression);
    void OnConnected(const char* version);
    void OnProtocolError();
    void OnMessage(Packet& p);

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
    afx_msg LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam);
    afx_msg void OnWindowManager();
    afx_msg void OnFileConnect();
    afx_msg LRESULT OnDisconnect(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnSockError(WPARAM wp, LPARAM lp);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg void OnViewCustomize();
    afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
    afx_msg void OnViewCaptionBar();
    afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
    afx_msg void OnOptions();
    afx_msg LRESULT OnInputSend(WPARAM wp, LPARAM lp);
    afx_msg LRESULT OnSwitchBuffer(WPARAM wp, LPARAM lp);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

    BOOL CreateDockingWindows();
    void SetDockingWindowIcons(BOOL bHiColorIcons);
    BOOL CreateCaptionBar();
};


