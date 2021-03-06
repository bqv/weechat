// WeechatEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "WeechatEditCtrl.h"

// CWeechatEditCtrl

IMPLEMENT_DYNAMIC(CWeechatEditCtrl, CWnd)

const LPCTSTR CWeechatEditCtrl::wndClassName = _T("WeechatEditEdit");

CWeechatEditCtrl::CWeechatEditCtrl()
{
    //Register window class
    RegisterWndClass();
}

CWeechatEditCtrl::~CWeechatEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CWeechatEditCtrl, CWnd)
END_MESSAGE_MAP()

LRESULT CWeechatEditCtrl::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWeechatEditCtrl *pThis;

    if (msg == WM_NCCREATE)
    {
        pThis = static_cast<CWeechatEditCtrl*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

        SetLastError(0);
        if (!SetWindowLongPtr(hwnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
        {
            if (GetLastError() != 0)
                return FALSE;
        }
    }
    else
    {
        pThis = reinterpret_cast<CWeechatEditCtrl*>(GetWindowLongPtr(hwnd, GWL_USERDATA));
    }

    if (pThis)
    {
        return pThis->WndProc(msg, wParam, lParam);
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CWeechatEditCtrl::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    /*
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        SetFocus();
        break;

    case WM_KILLFOCUS:
        HideCaret();
        DestroyCaret();
        break;

    case WM_SETFOCUS:
    {
        RECT r;
        GetClientRect(&r);
        // Create a solid black caret. 
        CreateSolidCaret(2, r.bottom - r.top);

        ShowCaret();
        Redraw();
    }

    return FALSE;

    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS | DLGC_WANTARROWS;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case 'V':
            if (0x8000 & GetKeyState(VK_CONTROL))
            {
                HANDLE h;
                wchar_t *cb;
                int len, slen;

                Delete();

                OpenClipboard();
                h = GetClipboardData(CF_UNICODETEXT);

                cb = (wchar_t*)GlobalLock(h);

                if (cb)
                {
                    memcpy(buf + (m_cursor + len) * sizeof(wchar_t), buf + m_cursor * sizeof(wchar_t), (slen - m_cursor) * sizeof(wchar_t));
                    memcpy(buf + m_cursor * sizeof(wchar_t), cb, len * sizeof(wchar_t));
                }

                GlobalUnlock(h);
                CloseClipboard();
                Redraw();
            }
            break;

        case VK_RIGHT:

            if (m_cursor - 1 >= MAXINPUTBUF || m_cursor >= (int)wcslen(buf))
                break;

            m_cursor++;

            if (!(GetKeyState(VK_SHIFT) & 0x8000))
                m_select = m_cursor;

            Redraw();
            break;

        case VK_TAB:
            PostMessageW(GetParent(), WM_NEXTDLGCTL, GetKeyState(VK_SHIFT) & 0x8000, FALSE);
            break;

        case VK_LEFT:
            if (m_cursor <= 0)
                break;

            m_cursor--;

            if (!(GetKeyState(VK_SHIFT) & 0x8000))
                m_select = m_cursor;

            Redraw();
            break;

        case VK_HOME:
            m_cursor = 0;

            if (!(GetKeyState(VK_SHIFT) & 0x8000))
                m_select = m_cursor;

            Redraw();
            break;

        case VK_END:
            m_cursor = wcslen(buf);

            if (!(GetKeyState(VK_SHIFT) & 0x8000))
                m_select = m_cursor;

            Redraw();
            break;

        case VK_DELETE:
            if (m_cursor >= (int)wcslen(buf))
            {
                Delete();
                Redraw();
                break;
            }

            if (m_select == m_cursor)
                m_select++;

            Delete();
            Redraw();
            break;

        case VK_BACK:

            if (m_cursor <= 0)
            {
                Delete();
                Redraw();
                break;
            }


            if (m_select == m_cursor)
                m_cursor--;


            Delete();
            Redraw();
        }

    }
    break;

    case WM_CHAR:
        if (wParam < VK_SPACE)
            break;


        Delete();

        if (wcslen(buf) + 1 < MAXINPUTBUF)
        {
            wmemmove(buf + (m_cursor + 1) * sizeof(wchar_t), buf + m_cursor * sizeof(wchar_t), wcslen(s->buf) - m_cursor);
            buf[m_cursor] = wParam;
            m_cursor++;
            m_select = m_cursor;
        }

        Redraw();

        break;

    case WM_ERASEBKGND:
        // no flickering
        return TRUE;

    case WM_PAINT:
    {
        HDC dc;
        PAINTSTRUCT paint;

        dc = BeginPaint(&paint);
        Draw(dc);
        EndPaint(&paint);

    }
    return TRUE;

    }
    */return TRUE;
}

void CWeechatEditCtrl::Redraw()
{
    HideCaret();

    CDC* pDC = GetDC();
    Draw(pDC);
    ReleaseDC(pDC);

    ShowCaret();
}

void CWeechatEditCtrl::Draw(CDC * pDC)
{
    RECT r, cr;

    GetClientRect(&cr);

    // draw selected rectangle FillRect()...

    CopyRect(&r, &cr);
    //DrawText(*pDC, buf, -1, &r, DT_LEFT | DT_TOP);

    /*
    if (m_cursor)
        DrawText(*pDC, buf, m_cursor, &r, DT_LEFT | DT_TOP | DT_CALCRECT);
    else
        r.right = cr.left;
    */

    if (GetFocus()->m_hWnd == m_hWnd)
    {
        if (r.right > cr.right)
            SetCaretPos({ cr.right, cr.top });
        else
            SetCaretPos({ r.right, cr.top });
    }
}

BOOL CWeechatEditCtrl::RegisterWndClass()
{
    WNDCLASS windowclass;
    HINSTANCE hInst = AfxGetInstanceHandle();

    //Check weather the class is registerd already
    if (!(::GetClassInfo(hInst, wndClassName, &windowclass)))
    {
        //If not then we have to register the new class
        windowclass.style = CS_DBLCLKS;// | CS_HREDRAW | CS_VREDRAW;
        windowclass.lpfnWndProc = CWeechatEditCtrl::WndProc;
        windowclass.cbClsExtra = windowclass.cbWndExtra = 0;
        windowclass.hInstance = hInst;
        windowclass.hIcon = NULL;
        windowclass.hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        windowclass.hbrBackground = ::GetSysColorBrush(COLOR_WINDOW);
        windowclass.lpszMenuName = NULL;
        windowclass.lpszClassName = wndClassName;

        if (!AfxRegisterClass(&windowclass))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;
}
