// NicklistView.cpp : implementation file
//

#include "stdafx.h"
#include "NicklistView.h"
#include "Main.h"

// CNicklistView

IMPLEMENT_DYNAMIC(CNicklistView, CDockablePane)

CNicklistView::CNicklistView()
{

}

CNicklistView::~CNicklistView()
{
}


BEGIN_MESSAGE_MAP(CNicklistView, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CNicklistView message handlers

int CNicklistView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rectDummy;
    rectDummy.SetRectEmpty();

    // Create view:
    const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | LVS_SMALLICON;

    if (!m_wndNicklistView.Create(dwViewStyle, rectDummy, this, 4))
    {
        TRACE0("Failed to create file view\n");
        return -1;      // fail to create
    }

    ///
    m_wndNicklistView.InsertItem(0, _T("User1"));
    m_wndNicklistView.InsertItem(1, _T("User2"));
    m_wndNicklistView.InsertItem(2, _T("User3"));
    m_wndNicklistView.InsertItem(3, _T("User4"));
    ///
    AdjustLayout();

    return 0;
}

void CNicklistView::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    AdjustLayout();
}

void CNicklistView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CListCtrl* pWndList = (CListCtrl*)&m_wndNicklistView;
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

void CNicklistView::AdjustLayout()
{
    if (GetSafeHwnd() == nullptr)
    {
        return;
    }

    CRect rectClient;
    GetClientRect(rectClient);

    m_wndNicklistView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + 1, rectClient.Width() - 2, rectClient.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}



