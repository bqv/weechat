// InputCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "InputCtrl.h"

// CInputCtrl

IMPLEMENT_DYNAMIC(CInputCtrl, CEdit)

CInputCtrl::CInputCtrl()
{
    //Register window class
    RegisterWndClass();
}

CInputCtrl::~CInputCtrl()
{
}

BEGIN_MESSAGE_MAP(CInputCtrl, CEdit)
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CInputCtrl message handlers

void CInputCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CWnd *pWnd = GetParent();

    if (nChar == VK_RETURN)
    {
        if (GetKeyState(VK_CONTROL) < 0)
        {
            //ReplaceSel(_T("\n"), TRUE);
        }
        else
        {
            if (pWnd) pWnd->PostMessage(WM_INPUT_SEND);
        }
    }

    /*
    int iCnt = GetLineCount();
    CDC* pdc = GetDC();
    CString text;
    GetWindowText(text);
    CSize szText = pdc->GetTextExtent(text);
    SetWindowPos(NULL, 0, 0, szText.cx, szText.cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    */

    CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CInputCtrl::RegisterWndClass()
{
    WNDCLASS windowclass;
    HINSTANCE hInst = AfxGetInstanceHandle();

    static const LPCTSTR wndClassName = _T("InputEdit");

    //Check weather the class is registerd already
    if (!(::GetClassInfo(hInst, wndClassName, &windowclass)))
    {
        //If not then we have to register the new class
        ::GetClassInfo(hInst, WC_EDIT, &windowclass);
        windowclass.lpszClassName = wndClassName;

        if (!AfxRegisterClass(&windowclass))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;

}
