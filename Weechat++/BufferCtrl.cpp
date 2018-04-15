// BufferCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BufferCtrl.h"


// CBufferCtrl

IMPLEMENT_DYNAMIC(CBufferCtrl, CRichEditCtrl)

CBufferCtrl::CBufferCtrl()
{
    //Register window class
    RegisterWndClass();
}

CBufferCtrl::~CBufferCtrl()
{
}


BEGIN_MESSAGE_MAP(CBufferCtrl, CRichEditCtrl)
END_MESSAGE_MAP()



// CBufferCtrl message handlers

BOOL CBufferCtrl::RegisterWndClass()
{
    WNDCLASS windowclass;
    HINSTANCE hInst = AfxGetInstanceHandle();

    static const LPCWSTR wndClassName = L"BufferEdit";

    //Check weather the class is registerd already
    if (!(::GetClassInfo(hInst, wndClassName, &windowclass)))
    {
        //If not then we have to register the new class
        ::GetClassInfo(hInst, L"RichEdit20W", &windowclass);
        windowclass.lpszClassName = wndClassName;

        if (!AfxRegisterClass(&windowclass))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;

}

