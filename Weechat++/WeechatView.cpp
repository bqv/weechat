
// WeechatView.cpp : implementation of the CWeechatView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Main.h"
#endif

#include "WeechatBuffer.h"
#include "WeechatView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWeechatView

IMPLEMENT_DYNCREATE(CWeechatView, CFormView)

BEGIN_MESSAGE_MAP(CWeechatView, CFormView)
    ON_MESSAGE(WM_INPUT_SEND, &CWeechatView::OnInputSend)
    ON_WM_SIZE()
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CWeechatView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CWeechatView construction/destruction

CWeechatView::CWeechatView()
    : CFormView(CWeechatView::IDD)
{
    m_szDesignTimeSize.SetSize(0, 0);
    
}

void CWeechatView::AddLine(CBufferLine& line)
{
    SendDlgItemMessage(IDC_BUFFER, BM_LINES, BUF_ADDLINE, (LPARAM)&line);
}

void CWeechatView::SetLines(CList<CBufferLine>& lines)
{
    SendDlgItemMessage(IDC_BUFFER, BM_LINES, BUF_SETLINES, (LPARAM)&lines);
}

CWeechatView::~CWeechatView()
{
}

void CWeechatView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCustomControlDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    DDX_Control(pDX, IDC_INPUT, m_inputCtrl);
    //}}AFX_DATA_MAP
}

BOOL CWeechatView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CWeechatView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);

    GetParentFrame()->RecalcLayout();
    ResizeParentToFit(FALSE);

    BOOL bOk = m_resizer.Hook(this);
    ASSERT(bOk);

    bOk = m_resizer.SetAnchor(IDC_INPUT, ANCHOR_HORIZONTALLY | ANCHOR_BOTTOM);
    ASSERT(bOk);

    bOk = m_resizer.SetAnchor(IDC_BUFFER, ANCHOR_ALL);
    ASSERT(bOk);

    bOk = m_resizer.SetAnchor(IDC_TOPIC, ANCHOR_HORIZONTALLY | ANCHOR_TOP);
    ASSERT(bOk);

    bOk = m_resizer.InvokeOnResized();
    ASSERT(bOk);

    CString sDebug = m_resizer.GetDebugInfo();
    OutputDebugString(sDebug);

    EnableScrollBarCtrl(SB_BOTH, FALSE);

    m_inputCtrl.ModifyStyle(WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN, 0);
    m_inputCtrl.SetCueBanner(_T("nick"));
    m_inputCtrl.SetFocus();
}

void CWeechatView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CWeechatBuffer* pBuffer = GetDocument();

    pBuffer->SetModifiedFlag(FALSE);
}

// CWeechatView printing


void CWeechatView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
    AFXPrintPreview(this);
#endif
}

BOOL CWeechatView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CWeechatView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CWeechatView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CWeechatView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
    // TODO: add customized printing code here
}

int CWeechatView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFormView::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_szDesignTimeSize.SetSize(lpCreateStruct->cx, lpCreateStruct->cy);

    return 0;
}

void CWeechatView::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
}


// CWeechatView diagnostics

#ifdef _DEBUG
void CWeechatView::AssertValid() const
{
	CFormView::AssertValid();
}

void CWeechatView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CWeechatBuffer* CWeechatView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWeechatBuffer)));
	return (CWeechatBuffer*)m_pDocument;
}
#endif //_DEBUG


// CWeechatView message handlers

LRESULT CWeechatView::OnInputSend(WPARAM wp, LPARAM lp)
{
    CString line;
    CString buffer = GetDocument()->m_pointer;
    CWnd *pWnd = theApp.m_pMainWnd;//GetParent();

    m_inputCtrl.GetWindowText(line);
    m_inputCtrl.SetWindowText(NULL);
    if (pWnd) pWnd->SendMessage(WM_INPUT_SEND, (WPARAM)&line, (LPARAM)&buffer);

    return LRESULT();
}
