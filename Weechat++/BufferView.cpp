// BufferView.cpp : implementation file
//

#include "stdafx.h"
#include "BufferView.h"
#include "Main.h"
#include "WeechatBuffer.h"

// CBufferView

IMPLEMENT_DYNAMIC(CBufferView, CDockablePane)

CBufferView::CBufferView()
{

}

CBufferView::~CBufferView()
{
}

BEGIN_MESSAGE_MAP(CBufferView, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(WM_UPDATE_BUFFERS, &CBufferView::OnUpdateBuffers)
END_MESSAGE_MAP()

// CBufferView message handlers

int CBufferView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create view:
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

    if (!m_wndBufferView.Create(dwViewStyle, rectDummy, this, 4))
    {
        TRACE0("Failed to create buffer view\n");
        return -1;      // fail to create
    }

    m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_MENU_IMAGES);
    m_wndToolBar.LoadToolBar(IDR_MENU_IMAGES, 0, 0, TRUE /* Is locked */);

    m_wndToolBar.CleanUpLockedImages();
    m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : IDR_MENU_IMAGES, 0, 0, TRUE /* Locked */);

    m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

    m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

    m_wndToolBar.SetOwner(this);

    // All commands will be routed via this control , not via the parent frame:
    m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

    AdjustLayout();

    return 0;
}

void CBufferView::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

void CBufferView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CListCtrl* pWndList = (CListCtrl*)&m_wndBufferView;
    ASSERT_VALID(pWndList);

    if (pWnd != pWndList)
    {
        CDockablePane::OnContextMenu(pWnd, point);
        return;
    }

    if (point != CPoint(-1, -1))
    {
        // Select clicked item:
        CPoint ptList = point;
        pWndList->ScreenToClient(&ptList);

        UINT flags = 0;
        int hListItem = pWndList->HitTest(ptList, &flags);
        if (hListItem != -1)
        {
            pWndList->SetHotItem(hListItem);
        }
    }

    pWndList->SetFocus();
    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

LRESULT CBufferView::OnUpdateBuffers(WPARAM wp, LPARAM lp)
{
    switch (wp)
    {
    case 0:
    {
        CList<CWeechatBuffer>* newBuffers = reinterpret_cast<CList<CWeechatBuffer>*>(lp);
        m_wndBufferView.DeleteAllItems();
        CMap<CString, LPCTSTR, HTREEITEM, HTREEITEM&> roots;
        POSITION pos = newBuffers->GetHeadPosition();
        while (pos != NULL)
        {
            CWeechatBuffer buffer = newBuffers->GetNext(pos);
            int i = 0;
            CList<CString> path;
            for (CString subpath = buffer.m_name.Tokenize(_T("."), i); i >= 0; subpath = buffer.m_name.Tokenize(_T("."), i))
            {
                path.AddTail(subpath);
                // 
                path.AddTail(buffer.m_name.Tokenize(_T(""), i));
                break;
                //
            }
            if (path.GetSize() == 1)
            {
                m_wndBufferView.InsertItem(buffer.m_name, 0, 0);
            }
            else if (path.GetSize() == 2)
            {
                POSITION pos = path.GetHeadPosition();
                CString pathHead = path.GetNext(pos);
                if (pathHead == "server")
                {
                    CString name = path.GetNext(pos);
                    if (!roots.PLookup(pathHead))
                    {
                        HTREEITEM root = m_wndBufferView.InsertItem(name, 1, 1);
                        roots.SetAt(name, root);
                    }
                }
                else
                {
                    HTREEITEM root;
                    if (roots.Lookup(pathHead, root))
                    {
                        m_wndBufferView.InsertItem(path.GetNext(pos), 0, 0, root);
                    }
                    else
                    {
                        root = m_wndBufferView.InsertItem(pathHead, 1, 1);
                        roots.SetAt(pathHead, root);
                        m_wndBufferView.InsertItem(path.GetNext(pos), 0, 0, root);
                    }
                }
            }
            else
            {
                m_wndBufferView.InsertItem(buffer.m_shortName, 0, 0);
            }
        }
        return LRESULT();
    }
    default:
        return LRESULT();
    }
}

void CBufferView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

    m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
    m_wndBufferView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}
