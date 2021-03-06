#pragma once

#include "WeechatBuffer.h"

class CBufferViewToolBar : public CMFCToolBar
{
    virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
    {
        CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
    }

    virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CBufferView : public CDockablePane
{
	DECLARE_DYNAMIC(CBufferView)

public:
	CBufferView();

    void AdjustLayout();

// Attributes
protected:
    CMap<HTREEITEM, HTREEITEM, CWeechatBuffer, CWeechatBuffer&> m_buffers;

    CTreeCtrl m_wndBufferView;
    CBufferViewToolBar m_wndToolBar;

// Implementation
public:
	virtual ~CBufferView();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg LRESULT OnUpdateBuffers(WPARAM wp, LPARAM lp);
    afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClickTree(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkTree(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};


