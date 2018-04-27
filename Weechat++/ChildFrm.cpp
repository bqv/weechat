
// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "Main.h"

#include "ChildFrm.h"
#include "WeechatBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
    ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
    // TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
    /*
    return m_wndSplitter.Create(this,
        2, 2,			// TODO: adjust the number of rows, columns
        CSize(10, 10),	// TODO: adjust the minimum pane size
        pContext);
    */return CMDIChildWndEx::OnCreateClient(lpcs, pContext);
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWndEx::PreCreateWindow(cs))
        return FALSE;

    cs.style &= ~FWS_ADDTOTITLE;

    return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
    CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
    CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame message handlers

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
    if (bActivate && pActivateWnd == this)
    {
        CWnd* parent = theApp.m_pMainWnd;
        CDocument* document = GetActiveDocument();
        ASSERT_KINDOF(CWeechatBuffer, document);
        CWeechatBuffer* buffer = (CWeechatBuffer*)document;
        CString pointer = buffer->m_pointer;
        parent->SendMessage(WM_SWITCH_BUFFER, (WPARAM)&pointer);
    }

    return CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
}

void CChildFrame::DoDataExchange(CDataExchange* pDX)
{
    CMDIChildWndEx::DoDataExchange(pDX);
}
