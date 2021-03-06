#pragma once


// CWeechatEditCtrl

class CWeechatEditCtrl : public CWnd
{
    DECLARE_DYNAMIC(CWeechatEditCtrl)

protected:
    DECLARE_MESSAGE_MAP()

    // Constructors
public:
    CWeechatEditCtrl();
    virtual ~CWeechatEditCtrl();

    // Attributes
public:
    static const LPCTSTR wndClassName;

protected:
    int m_select; // current selection position
    int m_cursor; // current cursor position

                  // Message handlers
protected:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    virtual void Redraw();
    virtual void Draw(CDC* pDC);

private:
    BOOL RegisterWndClass();
};


