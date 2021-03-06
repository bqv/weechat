
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Main.h"

#include "MainFrm.h"
#include "ConnectDlg.h"
#include "WeechatBuffer.h"
#include "BufferLine.h"
#include "Hash.h"
#include "CodePoint.h"
#include "Nicklist.h"
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
    ON_MESSAGE(WM_SWITCH_BUFFER, &CMainFrame::OnSwitchBuffer)
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
        m_status.Format(_T("Disconnected!"));
        m_relayConn.Close();
    }
}

void CMainFrame::OnConnected(const char* version)
{
    theApp.WriteProfileInt(REG_SECTION_LOGIN, REG_ENTRY_TYPE, m_relayType);
    theApp.WriteProfileString(REG_SECTION_LOGIN, REG_ENTRY_HOST, m_relayHost);
    theApp.WriteProfileInt(REG_SECTION_LOGIN, REG_ENTRY_PORT, m_relayPort);
    theApp.WriteProfileString(REG_SECTION_LOGIN, REG_ENTRY_PASS, m_relayPass);

    CString ver_str = UTF8TOCS(version);
    m_status.Format(_T("Connected: Weechat %s"), ver_str);
    SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
    m_wndStatusBar.SetPaneText(0, m_status, TRUE);
    
    m_relayHandler.hdatasync(HDATA_BUFFER, PTR_GUI_BUFFERS"(*)", {}, { BUFFER_NUMBER, BUFFER_NAME, BUFFER_FULL_NAME, BUFFER_SHORT_NAME, BUFFER_TITLE, BUFFER_TYPE }, { "*" }, { SYNC_BUFFERS, SYNC_UPGRADE }, [&](Packet* p) {
        if (p)
        {
            ProtocolAssert(p->objects.size() == 1);
            ProtocolAssert(std::visit(ID(), p->objects[0]) == hda_t::ID);
            hda_t hdata = std::get<hda_t>(p->objects[0]);
            ProtocolAssert(hdata.hpath == std::vector<std::string>({HDATA_BUFFER}));
            ProtocolAssert(hdata.keys.size() == 6);

            std::vector<std::pair<std::map<std::string, ptr_t>, std::map<std::string, obj_t>>> entries;
            for (const auto& entry : hdata.values)
            {
                int i = 0;
                std::map<std::string, ptr_t> entry_pointers;
                ProtocolAssert(entry.first.size() == 1);
                for (const auto& path : hdata.hpath)
                {
                    entry_pointers.insert({ path, { entry.first[i++] } });
                }

                int j = 0;
                std::map<std::string, obj_t> entry_values;
                ProtocolAssert(entry.second.size() == 6);
                for (const auto& key : hdata.keys)
                {
                    obj_t object = entry.second[j++];
                    const char *id = std::visit(ID(), object);
                    ProtocolAssert(id && key.second.compare(0, 3, id) == 0);
                    entry_values.insert({ key.first, object });
                }

                entries.push_back({ entry_pointers, entry_values });
            }

            CList<CWeechatBuffer> buffers;
            for (const auto& entry : entries)
            {
                CWeechatBuffer buffer;
                try
                {
                    buffer.m_pointer = entry.first.at(HDATA_BUFFER).data.c_str();
                    buffer.m_number = std::get<int_t>(entry.second.at(BUFFER_NUMBER)).data;
                    auto name = std::get<str_t>(entry.second.at(BUFFER_NAME)).data;
                    if (name)
                        buffer.m_name = UTF8TOCS(name.value().c_str());
                    auto fullName = std::get<str_t>(entry.second.at(BUFFER_FULL_NAME)).data;
                    if (fullName)
                        buffer.m_fullName = UTF8TOCS(fullName.value().c_str());
                    auto shortName = std::get<str_t>(entry.second.at(BUFFER_SHORT_NAME)).data;
                    if (shortName)
                        buffer.m_shortName = UTF8TOCS(shortName.value().c_str());
                    auto title = std::get<str_t>(entry.second.at(BUFFER_TITLE)).data;
                    if (title)
                        buffer.m_title = UTF8TOCS(title.value().c_str());
                    buffer.m_type = std::get<int_t>(entry.second.at(BUFFER_TYPE)).data;
                }
                catch (std::bad_variant_access&)
                {
                    ProtocolAssert(false);
                }
                catch (std::out_of_range&)
                {
                    ProtocolAssert(false);
                }
                buffers.AddTail(buffer);
            }

            m_wndBufferView.SendMessage(WM_UPDATE_BUFFERS, 0, (LPARAM)&buffers);
        }
        else
        {
            ProtocolAssert(false);
        }
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
        m_relayType = connectDialog.m_relayType;
        m_relayHost = connectDialog.m_relayHost;
        m_relayPort = connectDialog.m_relayPort;
        m_relayPass = connectDialog.m_relayPass;

        CStringA password = CSTOUTF8(m_relayPass);
        m_relayHandler.init(password.GetString(), true);

        switch (m_relayType)
        {
        case 0:
            if (m_relayConn.Plain(m_relayHost, m_relayPort) == FALSE)
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
            break;
        case 1:
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
            break;
        default:
            AfxThrowInvalidArgException();
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
    m_status.Format(_T("Disconnected"));
    m_relayConn.Close();
}

void CMainFrame::OnMessage(Packet& p)
{
    if (p.id)
    {
        std::string id = p.id.value();
        if (id.size() > 0 && id[0] == '_')
        {
            char code = Handler::RESERVED.at(id);
            switch (code)
            {
            case EVENT_BUFFER_OPENED:
                break;
            case EVENT_BUFFER_TYPE_CHANGED:
                break;
            case EVENT_BUFFER_MOVED:
                break;
            case EVENT_BUFFER_MERGED:
                break;
            case EVENT_BUFFER_UNMERGED:
                break;
            case EVENT_BUFFER_HIDDEN:
                break;
            case EVENT_BUFFER_UNHIDDEN:
                break;
            case EVENT_BUFFER_RENAMED:
                break;
            case EVENT_BUFFER_TITLE_CHANGED:
                break;
            case EVENT_BUFFER_LOCALVAR_ADDED:
                break;
            case EVENT_BUFFER_LOCALVAR_CHANGED:
                break;
            case EVENT_BUFFER_LOCALVAR_REMOVED:
                break;
            case EVENT_BUFFER_CLOSING:
                break;
            case EVENT_BUFFER_CLEARED:
                break;
            case EVENT_BUFFER_LINE_ADDED:
            {
                ProtocolAssert(p.objects.size() == 1);
                ProtocolAssert(std::visit(ID(), p.objects[0]) == hda_t::ID);
                hda_t hdata = std::get<hda_t>(p.objects[0]);
                ProtocolAssert(hdata.hpath == std::vector<std::string>({HDATA_LINE_DATA}));
                ProtocolAssert(hdata.keys.size() == 8);
                ProtocolAssert(hdata.values.size() == 1);

                std::pair<std::map<std::string, ptr_t>, std::map<std::string, obj_t>> entry;
                {
                    int i = 0;
                    std::map<std::string, ptr_t> entry_pointers;
                    ProtocolAssert(hdata.values[0].first.size() == 1);
                    for (const auto& path : hdata.hpath)
                    {
                        entry_pointers.insert({ path, { hdata.values[0].first[i++] } });
                    }

                    int j = 0;
                    std::map<std::string, obj_t> entry_values;
                    ProtocolAssert(hdata.values[0].second.size() == 8);
                    for (const auto& key : hdata.keys)
                    {
                        obj_t object = hdata.values[0].second[j++];
                        const char *id = std::visit(ID(), object);
                        ProtocolAssert(id && key.second.compare(0, 3, id) == 0);
                        entry_values.insert({ key.first, object });
                    }

                    entry = { entry_pointers, entry_values };
                }

                CBufferLine line;
                try
                {
                    line.pointer = entry.first.at(HDATA_LINE_DATA).data.c_str();
                    line.buffer = std::get<ptr_t>(entry.second.at(LINE_DATA_BUFFER)).data.c_str();
                    line.date = std::get<tim_t>(entry.second.at(LINE_DATA_DATE)).data;
                    line.datePrinted = std::get<tim_t>(entry.second.at(LINE_DATA_DATE_PRINTED)).data;
                    line.displayed = std::get<chr_t>(entry.second.at(LINE_DATA_DISPLAYED)).data;
                    line.highlight = std::get<chr_t>(entry.second.at(LINE_DATA_HIGHLIGHT)).data;
                    arr_t tags_array = std::get<arr_t>(entry.second.at(LINE_DATA_TAGS_ARRAY));
                    ProtocolAssert(strncmp(tags_array.type, str_t::ID, 3) == 0);
                    for (const auto& entry : tags_array.values)
                        line.tagsArray.AddTail(UTF8TOCS(std::get<str_t>(entry).data.value().c_str()));
                    auto prefix = std::get<str_t>(entry.second.at(LINE_DATA_PREFIX)).data;
                    if (prefix)
                        line.prefix = UTF8TOCS(prefix.value().c_str());
                    auto message = std::get<str_t>(entry.second.at(LINE_DATA_MESSAGE)).data;
                    if (message)
                        line.message = UTF8TOCS(message.value().c_str());
                }
                catch (std::bad_variant_access&)
                {
                    ProtocolAssert(false);
                }
                catch (std::out_of_range&)
                {
                    ProtocolAssert(false);
                }

                POSITION pos = theApp.GetFirstDocTemplatePosition();
                CDocTemplate* pT = theApp.GetNextDocTemplate(pos);
                pos = pT->GetFirstDocPosition();
                while (pos != NULL)
                {
                    CWeechatBuffer* pDoc = (CWeechatBuffer*)pT->GetNextDoc(pos);
                    if (line.buffer == pDoc->m_pointer)
                    {
                        pos = pDoc->GetFirstViewPosition();
                        while (pos != NULL)
                        {
                            CWeechatView* pView = (CWeechatView*)pDoc->GetNextView(pos);
                            pView->AddLine(line);
                            pView->UpdateWindow();
                        }
                        pDoc->UpdateAllViews(NULL);
                        break;
                    }
                }
                break;
            }
            case EVENT_NICKLIST:
                break;
            case EVENT_NICKLIST_DIFF:
                break;
            case EVENT_PONG:
                break;
            case EVENT_UPGRADE:
                break;
            case EVENT_UPGRADE_ENDED:
                break;
            default:
                TRACE(_T("Ignoring unsolicited message with id (%s)\n"), CString(id.c_str()));
                break;
            }
        }
        else
        {
            TRACE(_T("Ignoring unsolicited message with id (%s)\n"), CString(id.c_str()));
        }
    }
    else
    {
        TRACE(_T("Ignoring unsolicited message without id\n"));
    }
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
    CString line = *(CString*)wp;
    CStringA data = CSTOUTF8(line);
    CString buffer = *(CString*)lp;
    CStringA pointer = CSTOUTF8(buffer);

    m_relayHandler.input(pointer.GetString(), data.GetString());
    
    /*
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
    */

    return LRESULT();
}

LRESULT CMainFrame::OnSwitchBuffer(WPARAM wp, LPARAM lp)
{
    CString buffer = ((CWeechatBuffer*)wp)->m_pointer;
    CNicklist nicklist;
    if (m_nicklistCache.Lookup(buffer, nicklist))
    {
        m_wndNicklistView.SendMessage(WM_UPDATE_NICKLIST, NICKLIST_REPLACE, (LPARAM)&nicklist);
    }
    else
    {
        CStringA pointer = CSTOUTF8(buffer);

        m_relayHandler.nicklist(pointer.GetString(), [buffer, this](Packet* p) {
            if (p)
            {
                ProtocolAssert(p->objects.size() == 1);
                ProtocolAssert(std::visit(ID(), p->objects[0]) == hda_t::ID);
                hda_t hdata = std::get<hda_t>(p->objects[0]);
                ProtocolAssert(hdata.hpath == std::vector<std::string>({ HDATA_BUFFER, HPATH_NICKLIST_ITEM }));
                ProtocolAssert(hdata.keys.size() == 7);

                std::vector<std::pair<std::map<std::string, ptr_t>, std::map<std::string, obj_t>>> entries;
                for (const auto& entry : hdata.values)
                {
                    int i = 0;
                    std::map<std::string, ptr_t> entry_pointers;
                    ProtocolAssert(entry.first.size() == 2);
                    for (const auto& path : hdata.hpath)
                    {
                        entry_pointers.insert({ path,{ entry.first[i++] } });
                    }

                    int j = 0;
                    std::map<std::string, obj_t> entry_values;
                    ProtocolAssert(entry.second.size() == 7);
                    for (const auto& key : hdata.keys)
                    {
                        obj_t object = entry.second[j++];
                        const char *id = std::visit(ID(), object);
                        ProtocolAssert(id && key.second.compare(0, 3, id) == 0);
                        entry_values.insert({ key.first, object });
                    }

                    entries.push_back({ entry_pointers, entry_values });
                }

                CList<CNicklist> nicklists;
                for (const auto& entry : entries)
                {
                    try
                    {
                        char group = std::get<chr_t>(entry.second.at(NICKLIST_GROUP)).data;
                        switch (group)
                        {
                        case 0: // Nick
                        {
                            CNicklistItem nick;
                            nick.m_pointer = entry.first.at(HPATH_NICKLIST_ITEM).data.c_str();
                            nick.m_buffer = entry.first.at(HDATA_BUFFER).data.c_str();
                            nick.m_visible = std::get<chr_t>(entry.second.at(NICKLIST_VISIBLE)).data;
                            auto name = std::get<str_t>(entry.second.at(NICKLIST_NAME)).data;
                            if (name)
                                nick.m_name = UTF8TOCS(name.value().c_str());
                            auto colour = std::get<str_t>(entry.second.at(NICKLIST_COLOR)).data;
                            if (colour)
                                nick.m_colour = UTF8TOCS(colour.value().c_str());
                            auto prefix = std::get<str_t>(entry.second.at(NICKLIST_PREFIX)).data;
                            if (prefix)
                                nick.m_prefix = UTF8TOCS(prefix.value().c_str());
                            auto prefixColour = std::get<str_t>(entry.second.at(NICKLIST_PREFIX_COLOR)).data;
                            if (prefixColour)
                                nick.m_prefixColour = UTF8TOCS(prefixColour.value().c_str());
                            CNicklist* branch = &nicklists.GetTail();
                            while (branch->m_sublists.GetCount() > 0)
                            {
                                CArray<CNicklist>& subbranches = branch->m_sublists;
                                branch = &subbranches.GetAt(subbranches.GetSize() - 1);
                            }
                            branch->m_items.Add(nick);
                            break;
                        }
                        case 1: // Group
                        {
                            CNicklist nicklist;
                            nicklist.m_pointer = entry.first.at(HPATH_NICKLIST_ITEM).data.c_str();
                            nicklist.m_buffer = entry.first.at(HDATA_BUFFER).data.c_str();
                            nicklist.m_visible = std::get<chr_t>(entry.second.at(NICKLIST_VISIBLE)).data;
                            auto name = std::get<str_t>(entry.second.at(NICKLIST_NAME)).data;
                            if (name)
                                nicklist.m_name = UTF8TOCS(name.value().c_str());
                            if (nicklist.m_name == _T("root"))
                                nicklists.AddTail(nicklist);
                            else
                            {
                                const int level = std::get<int_t>(entry.second.at(NICKLIST_LEVEL)).data;
                                CNicklist* branch = &nicklists.GetTail();
                                for (int i = 1; i < level; ++i)
                                {
                                    CArray<CNicklist>& subbranches = branch->m_sublists;
                                    branch = &subbranches.GetAt(subbranches.GetSize() - 1);
                                }
                                branch->m_sublists.Add(nicklist);
                            }
                            break;
                        }
                        default:
                            ProtocolAssert(false);
                        }
                    }
                    catch (std::bad_variant_access&)
                    {
                        ProtocolAssert(false);
                    }
                    catch (std::out_of_range&)
                    {
                        ProtocolAssert(false);
                    }
                }

                ProtocolAssert(nicklists.GetCount() == 1);
                m_wndNicklistView.SendMessage(WM_UPDATE_NICKLIST, NICKLIST_REPLACE, (LPARAM)&nicklists.GetHead());
                m_nicklistCache.SetAt(buffer, nicklists.GetHead());
            }
            else
            {
                ProtocolAssert(false);
            }
        });
        m_wndNicklistView.SendMessage(WM_UPDATE_NICKLIST, NICKLIST_REPLACE, (LPARAM)&nicklist);
    }
    return LRESULT();
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
    //m_wndOutput.UpdateFonts();
}
