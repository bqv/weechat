#pragma once

class CNicklistView : public CDockablePane
{
    DECLARE_DYNAMIC(CNicklistView)

public:
    CNicklistView();

    void AdjustLayout();

    // Attributes
protected:

    CMFCListCtrl m_wndNicklistView;

    // Implementation
public:
    virtual ~CNicklistView();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

    DECLARE_MESSAGE_MAP()
};

