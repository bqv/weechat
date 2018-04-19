
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Main.h"

#include "MainFrm.h"
#include "ConnectDlg.h"
#include "WeechatBuffer.h"
#include "BufferLine.h"
#include "Relay/Packet.h"
#include "Relay/Types.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
    ON_MESSAGE(WM_SETMESSAGESTRING, &CMainFrame::OnSetMessageString)
    ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
    ON_WM_GETMINMAXINFO()
    ON_COMMAND(ID_FILE_CONNECT, &CMainFrame::OnFileConnect)
    ON_MESSAGE(WM_SOCK_CLOSED, &CMainFrame::OnDisconnect)
    ON_MESSAGE(WM_SOCK_ERROR, &CMainFrame::OnSockError)
    ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
    ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
    ON_COMMAND(ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar)
    ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar)
    ON_COMMAND(ID_TOOLS_OPTIONS, &CMainFrame::OnOptions)
    ON_MESSAGE(WM_INPUT_SEND, &CMainFrame::OnInputSend)
    ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
    : m_relayHandler(&m_relayConn, this), m_relayConn(&m_relayHandler, this)
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
    m_relayConn.Close();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

    BOOL bNameValid;

    CMDITabInfo mdiTabParams;
    mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_VS2005; // other styles available...
    mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
    mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
    mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
    mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
    EnableMDITabbedGroups(TRUE, mdiTabParams);

    if (!m_wndMenuBar.Create(this))
    {
        TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
    }

    m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

    // prevent the menu bar from taking the focus on activation
    CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    CString strToolBarName;
    bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
    ASSERT(bNameValid);
    m_wndToolBar.SetWindowText(strToolBarName);

    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);
    m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

    // Allow user-defined toolbars operations:
    InitUserToolbars(nullptr, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
    m_status.Format(_T("Disconnected"));

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


    // enable Visual Studio 2005 style docking window behavior
    CDockingManager::SetDockingMode(DT_SMART);
    // enable Visual Studio 2005 style docking window auto-hide behavior
    EnableAutoHidePanes(CBRS_ALIGN_ANY);

    // Create a caption bar:
    if (!CreateCaptionBar())
    {
        TRACE0("Failed to create caption bar\n");
        return -1;      // fail to create
    }
    m_wndCaptionBar.ShowWindow(SW_SHOW);
    RecalcLayout(FALSE);

    // Load menu item image (not placed on any standard toolbars):
    CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

    // create docking windows
    if (!CreateDockingWindows())
    {
        TRACE0("Failed to create docking windows\n");
        return -1;
    }

    m_wndBufferView.EnableDocking(CBRS_ALIGN_ANY);
    m_wndNicklistView.EnableDocking(CBRS_ALIGN_ANY);
    DockPane(&m_wndBufferView, AFX_IDW_DOCKBAR_LEFT);
    DockPane(&m_wndNicklistView, AFX_IDW_DOCKBAR_RIGHT);

    // set the visual manager and style
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
    CDockingManager::SetDockingMode(DT_SMART);

    // Enable toolbar and docking window menu replacement
    EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

    // enable quick (Alt+drag) toolbar customization
    CMFCToolBar::EnableQuickCustomization();

    if (CMFCToolBar::GetUserImages() == nullptr)
    {
        // load user-defined toolbar images
        if (m_UserImages.Load(_T(".\\UserImages.bmp")))
        {
            CMFCToolBar::SetUserImages(&m_UserImages);
        }
    }

    // enable menu personalization (most-recently used commands)
    // TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
    CList<UINT, UINT> lstBasicCommands;

    lstBasicCommands.AddTail(ID_FILE_NEW);
    lstBasicCommands.AddTail(ID_FILE_OPEN);
    lstBasicCommands.AddTail(ID_FILE_SAVE);
    lstBasicCommands.AddTail(ID_APP_EXIT);
    lstBasicCommands.AddTail(ID_EDIT_CUT);
    lstBasicCommands.AddTail(ID_EDIT_PASTE);
    lstBasicCommands.AddTail(ID_EDIT_UNDO);
    lstBasicCommands.AddTail(ID_APP_ABOUT);
    lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
    lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);

    //CMFCToolBar::SetBasicCommands(lstBasicCommands);

	return 0;
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
    if (!lParam && wParam == AFX_IDS_IDLEMESSAGE)
    {
        wParam = 0;
        lParam = (long)((LPCTSTR)m_status);
    }
    return CMDIFrameWndEx::OnSetMessageString(wParam, lParam);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

    cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
        | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;

	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
    BOOL bNameValid;
    
    // Create class view
    CString strNicklistView;
    bNameValid = strNicklistView.LoadString(IDS_NICKLIST_VIEW);
    ASSERT(bNameValid);
    if (!m_wndNicklistView.Create(strNicklistView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_NICKLISTVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create Class View window\n");
        return FALSE; // failed to create
    }

    // Create buffer view
    CString strBufferView;
    bNameValid = strBufferView.LoadString(IDS_BUFFER_VIEW);
    ASSERT(bNameValid);
    if (!m_wndBufferView.Create(strBufferView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_BUFFERVIEW, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create File View window\n");
        return FALSE; // failed to create
    }

    SetDockingWindowIcons(theApp.m_bHiColorIcons);
    return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
    /*
    HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndFileView.SetIcon(hFileViewIcon, FALSE);

    HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndClassView.SetIcon(hClassViewIcon, FALSE);

    HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

    HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);
    */
}

BOOL CMainFrame::CreateCaptionBar()
{
    if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
    {
        TRACE0("Failed to create caption bar\n");
        return FALSE;
    }

    BOOL bNameValid;

    CString strTemp, strTemp2;
    bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
    ASSERT(bNameValid);
    m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CMFCCaptionBar::ALIGN_LEFT, FALSE);
    bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
    ASSERT(bNameValid);
    m_wndCaptionBar.SetButtonToolTip(strTemp);

    bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
    ASSERT(bNameValid);
    m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);

    m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
    bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
    ASSERT(bNameValid);
    bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
    ASSERT(bNameValid);
    m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

    return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::ProtocolAssert(const bool expression)
{
    if (!expression)
    {
        AfxMessageBox(_T("Error reading data from the relay"));
        m_relayConn.Close();
    }
}

void CMainFrame::OnConnected(const char* version)
{
#ifdef _UNICODE
    CString ver_str = CA2W(version);
#else
    CString ver_str(version);
#endif
    m_status.Format(_T("Connected: Weechat %s"), ver_str);
    SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
    m_wndStatusBar.SetPaneText(0, m_status, TRUE);
    
    m_relayHandler.hdata(HDATA_BUFFER, GUI_BUFFERS"(*)", {}, { BUFFER_NUMBER, BUFFER_NAME, BUFFER_FULLNAME, BUFFER_SHORTNAME, BUFFER_TITLE, BUFFER_TYPE }, [&](Packet* p) {
        if (p)
        {
            CList<CWeechatBuffer> buffers;
            hda_t hdata = std::get<hda_t>(p->objects[0]);
            ProtocolAssert(hdata.hpath == std::vector<std::string>({HDATA_BUFFER}));
            ProtocolAssert(hdata.keys.size() == 6);
            ProtocolAssert(hdata.keys[0] == std::pair<std::string, std::string>({BUFFER_NUMBER,int_t::ID}));
            ProtocolAssert(hdata.keys[1] == std::pair<std::string, std::string>({BUFFER_NAME,str_t::ID}));
            ProtocolAssert(hdata.keys[2] == std::pair<std::string, std::string>({BUFFER_FULLNAME,str_t::ID}));
            ProtocolAssert(hdata.keys[3] == std::pair<std::string, std::string>({BUFFER_SHORTNAME,str_t::ID}));
            ProtocolAssert(hdata.keys[4] == std::pair<std::string, std::string>({BUFFER_TITLE,str_t::ID}));
            ProtocolAssert(hdata.keys[5] == std::pair<std::string, std::string>({BUFFER_TYPE,int_t::ID}));

            for (const auto& entry : hdata.values)
            {
                ProtocolAssert(entry.first.size() == 1);
                ProtocolAssert(entry.second.size() == 6);

                CWeechatBuffer buffer;
                buffer.m_pointer = entry.first[0].c_str();
                buffer.m_number = std::get<int_t>(entry.second[0]).data;
                auto name = std::get<str_t>(entry.second[1]).data;
                if (name)
                    buffer.m_name = name.value().c_str();
                auto fullName = std::get<str_t>(entry.second[2]).data;
                if (fullName)
                    buffer.m_fullName = fullName.value().c_str();
                auto shortName = std::get<str_t>(entry.second[2]).data;
                if (shortName)
                    buffer.m_shortName = shortName.value().c_str();
                auto title = std::get<str_t>(entry.second[2]).data;
                if (title)
                    buffer.m_title = title.value().c_str();
                buffer.m_type = std::get<int_t>(entry.second[5]).data;
                buffers.AddTail(buffer);
            }

            m_wndBufferView.SendMessage(WM_UPDATE_BUFFERS, 0, (LPARAM)&buffers);
        }
        else
        {
            ProtocolAssert(false);
        }
        /*
    POSITION pos = theApp.GetFirstDocTemplatePosition();
    CDocTemplate* pT = theApp.GetNextDocTemplate(pos);
    CWeechatBuffer* pDoc = (CWeechatBuffer*)pT->OpenDocumentFile(NULL);
        CFrameWnd* pFrame = MDIGetActive();
        ASSERT(pFrame);
        CDocument* pActiveDoc = pFrame->GetActiveDocument();
        ASSERT(pActiveDoc->IsKindOf(RUNTIME_CLASS(CWeechatBuffer)));
        CWeechatBuffer* pActiveBuffer = (CWeechatBuffer*)pActiveDoc;

        CBufferLine line;
        line.message = szTemp;
        pActiveBuffer->AddLine(line);
        AfxMessageBox(szTemp);
        */
    });
}


// CMainFrame message handlers

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    lpMMI->ptMinTrackSize.x = 300;
    lpMMI->ptMinTrackSize.y = 150;
}

void CMainFrame::OnWindowManager()
{
    ShowWindowsDialog();
}

void CMainFrame::OnFileConnect()
{
    CConnectDlg connectDialog(this);

    INT_PTR nRet = -1;
    nRet = connectDialog.DoModal();

    if (nRet == IDOK)
    {
        m_relayHost = connectDialog.m_relayHost;
        m_relayPort = connectDialog.m_relayPort;
        m_relayPass = connectDialog.m_relayPass;

#ifdef _UNICODE
        CStringA password = CW2A(m_relayPass);
#else
        CStringA password(m_relayPass);
#endif
        m_relayHandler.init(password.GetString());

        if (m_relayConn.Ssl(m_relayHost, m_relayPort) == FALSE)
        {
            DWORD hResult = GetLastError();
            if (hResult != WSAEWOULDBLOCK)
            {
                LPTSTR messageBuffer = nullptr;
                size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, hResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&messageBuffer, 0, NULL);
                CString message(messageBuffer, size);
                LocalFree(messageBuffer);
                AfxMessageBox(message);
            }
        }
    }
}

LRESULT CMainFrame::OnDisconnect(WPARAM wp, LPARAM lp)
{
    int* pResult = (int*)wp;
    m_status.Format(_T("Disconnected!"));
    return TRUE;
}

void CMainFrame::OnProtocolError()
{
    AfxMessageBox(_T("Protocol Error. Disconnecting!"));
    m_relayConn.Close();
}

LRESULT CMainFrame::OnSockError(WPARAM wp, LPARAM lp)
{
    int* pResult = (int*)wp;

    if (*pResult != 0)
    {
        LPTSTR messageBuffer = nullptr;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, *pResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&messageBuffer, 0, NULL);
        CString message(messageBuffer, size);
        LocalFree(messageBuffer);
        CString text;
        text.Format(_T("%d: "), *pResult);
        text += message;
        AfxMessageBox(text);
    }
    return TRUE;
}

void CMainFrame::OnViewCustomize()
{
    CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
    pDlgCust->EnableUserDefinedToolbars();
    pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM lp)
{
    LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp, lp);
    if (lres == 0)
    {
        return 0;
    }

    CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
    ASSERT_VALID(pUserToolbar);

    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
    return lres;
}

void CMainFrame::OnViewCaptionBar()
{
    m_wndCaptionBar.ShowWindow(m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW);
    RecalcLayout(FALSE);
}

void CMainFrame::OnUpdateViewCaptionBar(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
}

void CMainFrame::OnOptions()
{
}


BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
    // base class does the real work

    if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }


    // enable customization button for all user toolbars
    BOOL bNameValid;
    CString strCustomize;
    bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    for (int i = 0; i < iMaxUserToolbars; i++)
    {
        CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
        if (pUserToolbar != nullptr)
        {
            pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
        }
    }

    return TRUE;
}

LRESULT CMainFrame::OnInputSend(WPARAM wp, LPARAM lp)
{
#ifdef _UNICODE
    CStringW line = *(CStringW*)wp + _T("\n");
    CStringA data = CW2A(line);
#else
    CStringA data = *(CStringA*)wp + _T("\n");
#endif

    int result = m_relayConn.Send(data.GetString(), data.GetLength());
    if (result == SOCKET_ERROR)
    {
        result = GetLastError();
        LPTSTR messageBuffer = nullptr;
        size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&messageBuffer, 0, NULL);
        CString message(messageBuffer, size);
        LocalFree(messageBuffer);
        CString text;
        text.Format(_T("%d: "), result);
        text += message;
        AfxMessageBox(text);
    }

    return LRESULT();
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
    //m_wndOutput.UpdateFonts();
}
