#pragma once

#include "Nicklist.h"

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
    void SetNicks(CNicklist& nicklist);
    void ApplyDiff(/*CNicklistDiff& diff*/);
    virtual ~CNicklistView();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg LRESULT OnUpdateNicklist(WPARAM wp, LPARAM lp);

    DECLARE_MESSAGE_MAP()
};

