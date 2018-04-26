#pragma once

#include <memory>

#include "Usp10.h"
#include "BufferLine.h"

class DisplayElement
{
protected:
    DisplayElement(const CRect rect);

public:
    CRect rect;

    virtual CRect draw(CDC* pDC, SCRIPT_DIGITSUBSTITUTE& sds) = 0;
};

class TextElement : public DisplayElement
{
public:
    TextElement(const CString text, const CRect rect, BOOL multiLine);

    const CWeechatString text;
    BOOL multiline;

    virtual CRect draw(CDC* pDC, SCRIPT_DIGITSUBSTITUTE& sds);
};

class BufferDocument
{
public:
    CList<std::shared_ptr<DisplayElement>> elements;
    CRect dimensions;
    LOGFONT font;
};

// CBufferCtrl

class CBufferCtrl : public CWnd
{
    DECLARE_DYNAMIC(CBufferCtrl)

protected:
    DECLARE_MESSAGE_MAP()

    // Constructors
public:
    CBufferCtrl();
    virtual ~CBufferCtrl();

    // Attributes
public:
    static const LPCTSTR wndClassName;

    void SetLines(CList<CBufferLine>& lines);
    void AddLine(CBufferLine& line);

protected:
    const TCHAR* m_timeFormat = _T("%H:%M:%S");
    SCRIPT_DIGITSUBSTITUTE m_sds;
    CList<CBufferLine> m_lines;
    BufferDocument m_document;
    bool m_follow = true;
    int m_timeWidth = 0;
    int m_prefixWidth = 0;
    int m_lineHeight = 1;
    int m_margin = 5;

    // Message handlers
protected:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLines(WPARAM wParam, LPARAM lParam);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg UINT OnGetDlgCode();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnNcDestroy();

    void RegenerateDocument(CDC* pDC);
    void AppendDocument(CDC* pDC, const CBufferLine& line);
    void RedrawVisibleElements(CDC* pDC);

private:
    BOOL RegisterWndClass();
};
