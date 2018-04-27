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

void CNicklistView::SetNicks(CNicklist& nicklist)
{
    for (int i = 0; i < nicklist.m_items.GetCount(); ++i)
    {
        const CNicklistItem& item = nicklist.m_items[i];
        m_wndNicklistView.InsertItem(m_wndNicklistView.GetItemCount(), item.m_prefix + item.m_name);
    }
    for (int i = 0; i < nicklist.m_sublists.GetCount(); ++i)
    {
        SetNicks(nicklist.m_sublists[i]);
    }
}

BEGIN_MESSAGE_MAP(CNicklistView, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(WM_UPDATE_NICKLIST, &CNicklistView::OnUpdateNicklist)
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

LRESULT CNicklistView::OnUpdateNicklist(WPARAM wp, LPARAM lp)
{
    switch (wp)
    {
    case NICKLIST_REPLACE:
    {
        CNicklist& nicklist = *(CNicklist*)lp;
        m_wndNicklistView.DeleteAllItems();
        m_wndNicklistView.RemoveAllGroups();
        SetNicks(nicklist);
        break;
    }
    case NICKLIST_DIFF:
        break;
    default:
        AfxThrowInvalidArgException();
        break;
    }

    return TRUE;
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



