
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

void CWeechatView::AddLine(CBufferLine line)
{
    m_lines.AddTail(line);
}

void CWeechatView::SetLines(CList<CBufferLine>& lines)
{
    m_lines.RemoveAll();
    for (POSITION pos = lines.GetHeadPosition(); pos != NULL; )
        m_lines.AddTail(lines.GetNext(pos));
}

CWeechatView::~CWeechatView()
{
}

void CWeechatView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCustomControlDlg)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    DDX_Control(pDX, IDC_BUFFER, m_bufferCtrl);
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
    ResizeParentToFit();

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

    CStringA cell3width;
    cell3width.Format("%d", m_twipsWidth);

    CStringA rtfDocument;
    rtfDocument += "{\\rtf1\\ansi\\deff0"; // RTF Header
    rtfDocument += "{{\\colortbl;\\red255\\green255\\blue255;\\red255\\green255\\blue255;}"; // Colour Table
    rtfDocument += "{\\fonttbl {\\f0 Courier New;}}}"; // Font Table
    rtfDocument += "\\fs18"; // Font Size
    for (POSITION pos = m_lines.GetHeadPosition(); pos; )
    {
        #define NOCELLBORDERS "\\clbrdrt\\brdrw1\\brdrcf0\\brdrs\\clbrdrl\\brdrw1\\brdrcf0\\brdrs\\clbrdrb\\brdrw1\\brdrcf0\\brdrs\\clbrdrr\\brdrw1\\brdrcf0\\brdrs"
        #define NOROWBORDERS "\\trbrdrt\\brdrw1\\brdrcf0\\brdrs\\trbrdrl\\brdrw1\\brdrcf0\\brdrs\\trbrdrb\\brdrw1\\brdrcf0\\brdrs\\trbrdrr\\brdrw1\\brdrcf0\\brdrs"
        const CBufferLine& line = m_lines.GetNext(pos);
        CStringA timeStr = CT2A(line.date.Format(_T("%H:%M:%S")));
        CStringA prefix = line.prefix.GetRtf();
        CStringA message = line.message.GetRtf();
        
        rtfDocument += "\\trowd\\trgaph144\\trautofit1" NOROWBORDERS; // Row Header
        rtfDocument += NOCELLBORDERS "\\clftsWidth1\\cellx1200"; // Cell Declarations
        rtfDocument += NOCELLBORDERS "\\clftsWidth1\\cellx2000"; // Cell Declarations
        rtfDocument += NOCELLBORDERS "\\clftsWidth1\\cellx" + cell3width; // Cell Declarations
        rtfDocument += "\\pard\\intbl " + timeStr + "\\cell"; // Cell 1
        rtfDocument += "\\pard\\intbl " + prefix + "\\cell"; // Cell 2
        rtfDocument += "\\pard\\intbl " + message + "\\cell"; // Cell 3
        rtfDocument += "\\row"; // Row Footer
    }
    rtfDocument += "}"; // RTF Footer
    m_bufferCtrl.ReplaceAllRtf(rtfDocument);
    m_bufferCtrl.SendMessage(WM_USER + 232, 0, 0);
    int nBegin = m_bufferCtrl.GetTextLength();
    m_bufferCtrl.SetSel(nBegin, nBegin);
    GetDocument()->SetModifiedFlag(FALSE);
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
    if (::IsWindow(m_bufferCtrl)) // Don't bother until buffer richedit is initialized
    {
        RECT clientRect;
        m_bufferCtrl.GetClientRect(&clientRect);
        CDC* dc = GetDC();
        double ppi = GetDeviceCaps(*dc, LOGPIXELSX);
        double pixels = clientRect.right - clientRect.left;
        double twipsWidth = pixels * 1440.0 / ppi;
        ReleaseDC(dc);

        m_twipsWidth = twipsWidth;
        OnUpdate(NULL, 0, 0);
    }
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
    CWnd *pWnd = theApp.m_pMainWnd;//GetParent();

    m_inputCtrl.GetWindowText(line);
    m_inputCtrl.SetWindowText(NULL);
    if (pWnd) pWnd->SendMessage(WM_INPUT_SEND, (WPARAM)&line);

    return LRESULT();
}
