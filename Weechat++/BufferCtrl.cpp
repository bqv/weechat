// BufferCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BufferCtrl.h"
#include "Main.h"

#pragma comment(lib, "Usp10.lib")

// CBufferCtrl

IMPLEMENT_DYNAMIC(CBufferCtrl, CWnd)

const LPCTSTR CBufferCtrl::wndClassName = _T("BufferEdit");

CBufferCtrl::CBufferCtrl()
{
    //Register window class
    RegisterWndClass();
}

CBufferCtrl::~CBufferCtrl()
{
}

BEGIN_MESSAGE_MAP(CBufferCtrl, CWnd)
    ON_MESSAGE(BM_LINES, &CBufferCtrl::OnLines)
    ON_WM_CREATE()
    ON_WM_LBUTTONDOWN()
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_GETDLGCODE()
    ON_WM_KEYDOWN()
    ON_WM_CHAR()
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_WM_MOUSEWHEEL()
    ON_WM_SIZING()
    ON_WM_SIZE()
    ON_WM_VSCROLL()
    ON_WM_SETTINGCHANGE()
    ON_WM_DESTROY()
    ON_WM_NCDESTROY()
END_MESSAGE_MAP()

void CBufferCtrl::SetLines(CList<CBufferLine>& lines)
{
    CDC memDC;
    memDC.CreateCompatibleDC(NULL);

    CFont font, oldFont;
    font.CreateFontIndirect(&theApp.settings.font);
    oldFont.Attach(memDC.SelectObject(font.Detach()));

    m_lines.RemoveAll();
    for (POSITION pos = lines.GetHeadPosition(); pos != NULL; )
    {
        CBufferLine& line = lines.GetNext(pos);
        if (::IsWindow(m_hWnd))
        {
            const SIZE* size;
            HRESULT hr;
            SCRIPT_STRING_ANALYSIS ssa;

            CString timeStr = line.date.Format(m_timeFormat);
            if (!timeStr.IsEmpty())
            {
                hr = ScriptStringAnalyse(memDC, timeStr.GetString(), timeStr.GetLength(), (int)timeStr.GetLength() * 3 / 2 + 16, DEFAULT_CHARSET, SSA_GLYPHS, 0, NULL, NULL, NULL, NULL, NULL, &ssa);
                if (FAILED(hr)) { AfxDebugBreak(); AfxThrowInvalidArgException(); }
                size = ScriptString_pSize(ssa);
                m_timeWidth = max(m_timeWidth, size->cx);
                m_lineHeight = max(m_lineHeight, size->cy);
                ScriptStringFree(&ssa);
            }

            CString prefixStr = line.prefix.GetPlain();
            if (!prefixStr.IsEmpty())
            {
                hr = ScriptStringAnalyse(memDC, prefixStr.GetString(), prefixStr.GetLength(), (int)prefixStr.GetLength() * 3 / 2 + 16, DEFAULT_CHARSET, SSA_GLYPHS, 0, NULL, NULL, NULL, NULL, NULL, &ssa);
                if (FAILED(hr)) { AfxDebugBreak(); AfxThrowInvalidArgException(); }
                size = ScriptString_pSize(ssa);
                m_prefixWidth = max(m_prefixWidth, size->cx);
                m_lineHeight = max(m_lineHeight, size->cy);
                ScriptStringFree(&ssa);
            }
        }
        m_lines.AddTail(line);
    }

    font.Attach(memDC.SelectObject(oldFont.Detach()));

    m_drawingTasks.run([this]() {
        SCROLLINFO siv;
        siv.cbSize = sizeof(SCROLLINFO);
        GetScrollInfo(SB_VERT, &siv);

        CRect clientRect;
        GetClientRect(&clientRect);

        CDC memDC;
        memDC.CreateCompatibleDC(NULL);

        CFont font, oldFont;
        font.CreateFontIndirect(&theApp.settings.font);
        oldFont.Attach(memDC.SelectObject(font.Detach()));

        std::shared_ptr<BufferDocument> document = std::make_shared<BufferDocument>();
        RegenerateDocument(&memDC, clientRect, document);
        concurrency::send(m_document, document);

        font.Attach(memDC.SelectObject(oldFont.Detach()));

        siv.nMin = 0;
        siv.nMax = document->dimensions.Height();
        siv.nPage = clientRect.Height();
        if (m_follow)
            siv.nPos = siv.nMax - siv.nPage;
        SetScrollInfo(SB_VERT, &siv, TRUE);

        PostMessage(WM_PAINT);
        Invalidate(FALSE);
    });
}

void CBufferCtrl::AddLine(CBufferLine& line)
{
    CDC memDC;
    memDC.CreateCompatibleDC(NULL);
    const SIZE* size;

    CFont font, oldFont;
    font.CreateFontIndirect(&theApp.settings.font);
    oldFont.Attach(memDC.SelectObject(font.Detach()));

    HRESULT hr;
    SCRIPT_STRING_ANALYSIS ssa;

    CString timeStr = line.date.Format(m_timeFormat);
    if (!timeStr.IsEmpty())
    {
        hr = ScriptStringAnalyse(memDC, timeStr.GetString(), timeStr.GetLength(), (int)timeStr.GetLength() * 3 / 2 + 16, DEFAULT_CHARSET, SSA_FIT, 0, NULL, NULL, NULL, NULL, NULL, &ssa);
        if (FAILED(hr)) { AfxDebugBreak(); AfxThrowInvalidArgException(); }
        size = ScriptString_pSize(ssa);
        m_timeWidth = max(m_timeWidth, size->cx);
        ScriptStringFree(&ssa);
    }

    CString prefixStr = line.prefix.GetPlain();
    if (!prefixStr.IsEmpty())
    {
        hr = ScriptStringAnalyse(memDC, prefixStr.GetString(), prefixStr.GetLength(), (int)prefixStr.GetLength() * 3 / 2 + 16, DEFAULT_CHARSET, SSA_FIT, 0, NULL, NULL, NULL, NULL, NULL, &ssa);
        if (FAILED(hr)) { AfxDebugBreak(); AfxThrowInvalidArgException(); }
        size = ScriptString_pSize(ssa);
        m_prefixWidth = max(m_prefixWidth, size->cx);
        ScriptStringFree(&ssa);
    }

    m_lines.AddTail(line);

    font.Attach(memDC.SelectObject(oldFont.Detach()));

    m_drawingTasks.run([line, this]() {
        SCROLLINFO siv;
        siv.cbSize = sizeof(SCROLLINFO);
        GetScrollInfo(SB_VERT, &siv);

        CRect clientRect;
        GetClientRect(&clientRect);

        CDC memDC;
        memDC.CreateCompatibleDC(NULL);

        CFont font, oldFont;
        font.CreateFontIndirect(&theApp.settings.font);
        oldFont.Attach(memDC.SelectObject(font.Detach()));

        std::shared_ptr<BufferDocument> document = concurrency::receive(m_document);
        AppendDocument(&memDC, clientRect, document, line);
        concurrency::send(m_document, document);

        font.Attach(memDC.SelectObject(oldFont.Detach()));

        siv.nMin = 0;
        siv.nMax = document->dimensions.Height();
        siv.nPage = clientRect.Height();
        if (m_follow)
            siv.nPos = siv.nMax - siv.nPage;
        SetScrollInfo(SB_VERT, &siv, TRUE);

        PostMessage(WM_PAINT);
        Invalidate(FALSE);
    });
}

LRESULT CBufferCtrl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWnd* pWnd;

    pWnd = CWnd::FromHandlePermanent(hWnd);
    if (pWnd == NULL) {
        pWnd = new CBufferCtrl();
        pWnd->Attach(hWnd);
    }
    ASSERT(pWnd->m_hWnd == hWnd);
    ASSERT(pWnd == CWnd::FromHandlePermanent(hWnd));
    return AfxCallWndProc(pWnd, hWnd, msg, wParam, lParam);
}

LRESULT CBufferCtrl::OnLines(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case BUF_ADDLINE:
        AddLine(*(CBufferLine*)lParam);
        return TRUE;
    case BUF_SETLINES:
        SetLines(*(CList<CBufferLine>*)lParam);
        return TRUE;
    default:
        return FALSE;
    }
}

int CBufferCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    VERIFY(ScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT, &m_sds) == S_OK);

    return CWnd::OnCreate(lpCreateStruct);
}

void CBufferCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetFocus();
}

void CBufferCtrl::OnKillFocus(CWnd * pNewWnd)
{
    HideCaret();
    DestroyCaret();
}

void CBufferCtrl::OnSetFocus(CWnd * pOldWnd)
{
    RECT r;
    GetClientRect(&r);
    // Create a solid black caret. 
    CreateSolidCaret(2, r.bottom - r.top);

    ShowCaret();
    //Redraw();
}

UINT CBufferCtrl::OnGetDlgCode()
{
    return DLGC_WANTALLKEYS | DLGC_WANTARROWS;
}

void CBufferCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    //switch (wParam)
    //{
    //}
}

void CBufferCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

BOOL CBufferCtrl::OnEraseBkgnd(CDC * pDC)
{
    std::shared_ptr<BufferDocument> document = concurrency::receive(m_document);

    CRect rcClient;
    GetClientRect(rcClient);

    if (document->dimensions.Height() < rcClient.Height())
        pDC->FillSolidRect(rcClient, GetSysColor(COLOR_WINDOW));

    return TRUE;
}

void CBufferCtrl::OnPaint()
{
    CPaintDC dc(this);

    //Draw Items needed to be drawn
    RedrawVisibleElements(&dc);
}

BOOL CBufferCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (zDelta < 0)
        OnVScroll(SB_LINEDOWN, abs(zDelta) / m_lineHeight, NULL);
    else
        OnVScroll(SB_LINEUP, abs(zDelta) / m_lineHeight, NULL);

    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CBufferCtrl::OnSizing(UINT nSide, LPRECT lpRect)
{
    // The window size is changing; cancel any existing drawing tasks.
    m_drawingTasks.cancel();
}

void CBufferCtrl::OnSize(UINT nType, int cx, int cy)
{
    // The window size has changed; cancel any existing drawing tasks.
    //m_drawingTasks.cancel();
    // Wait for any existing tasks to finish.
    //m_drawingTasks.wait();

    m_drawingTasks.run([this]() {
        SCROLLINFO siv;
        siv.cbSize = sizeof(SCROLLINFO);
        GetScrollInfo(SB_VERT, &siv);

        CRect clientRect;
        GetClientRect(&clientRect);

        CDC memDC;
        memDC.CreateCompatibleDC(NULL);

        CFont font, oldFont;
        font.CreateFontIndirect(&theApp.settings.font);
        oldFont.Attach(memDC.SelectObject(font.Detach()));

        std::shared_ptr<BufferDocument> document = std::make_shared<BufferDocument>();
        RegenerateDocument(&memDC, clientRect, document);
        concurrency::send(m_document, document);

        font.Attach(memDC.SelectObject(oldFont.Detach()));

        siv.nMin = 0;
        siv.nMax = document->dimensions.Height();
        siv.nPage = clientRect.Height();
        if (m_follow)
            siv.nPos = siv.nMax - siv.nPage;
        SetScrollInfo(SB_VERT, &siv, TRUE);

        PostMessage(WM_PAINT);
        Invalidate(FALSE);
    });
}

void CBufferCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
    if ((GetStyle() & WS_VSCROLL) == 0)
        return;

    // Get the minimum and maximum scroll-bar positions.
    int minpos;
    int maxpos;
    GetScrollRange(SB_VERT, &minpos, &maxpos);
    maxpos = GetScrollLimit(SB_VERT);

    // Get the current position of scroll box.
    int curpos = GetScrollPos(SB_VERT);
    int oldPos = curpos;

    // Determine the new position of scroll box.
    switch (nSBCode)
    {
    case SB_TOP:      // Scroll to the top.
        curpos = minpos;
        break;

    case SB_BOTTOM:      // Scroll to the bottom.
        curpos = maxpos;
        break;

    case SB_ENDSCROLL:   // End scroll.
        break;

    case SB_LINEUP:      // Scroll up.
        if (curpos > minpos)
            curpos -= ++nPos * m_lineHeight;
        break;

    case SB_LINEDOWN:   // Scroll down.
        if (curpos < maxpos)
            curpos += ++nPos * m_lineHeight;
        break;

    case SB_PAGEUP:    // Scroll one page up.
    {
        // Get the page size. 
        SCROLLINFO info;
        GetScrollInfo(SB_VERT, &info, SIF_ALL);

        if (curpos > minpos)
            curpos = max(minpos, curpos - (int)info.nPage);
        break;
    }

    case SB_PAGEDOWN:      // Scroll one page down.
    {
        // Get the page size. 
        SCROLLINFO info;
        GetScrollInfo(SB_VERT, &info, SIF_ALL);

        if (curpos < maxpos)
            curpos = min(maxpos, curpos + (int)info.nPage);
        break;
    }

    case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
        curpos = nPos;      // of the scroll box at the end of the drag operation.
        break;

    case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
        curpos = nPos;     // position that the scroll box has been dragged to.
        break;
    }

    int nScrolled = oldPos - curpos;

    if (nScrolled != 0)
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        
        ScrollWindowEx(0, m_lineHeight * nScrolled, rcClient, rcClient, NULL, NULL, MAKELONG(SW_SMOOTHSCROLL, 1000));
    }

    m_follow = (maxpos - curpos < m_lineHeight);

    SetScrollPos(SB_VERT, curpos);
}

void CBufferCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    VERIFY(ScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT, &m_sds) == S_OK);
}

void CBufferCtrl::OnDestroy()
{
    // The window is being destroyed; cancel any existing drawing tasks.
    m_drawingTasks.cancel();
    // Wait for any existing tasks to finish.
    m_drawingTasks.wait();
}

void CBufferCtrl::OnNcDestroy()
{
    delete this;
}

void CBufferCtrl::RegenerateDocument(CDC* pDC, const CRect clientRect, std::shared_ptr<BufferDocument> document)
{
    document->elements.RemoveAll();
    document->dimensions.SetRectEmpty();

    POSITION pos = m_lines.GetHeadPosition();
    const int count = m_lines.GetSize();
    int i = 0;
    while (pos != NULL)
    {
        const CBufferLine& line = m_lines.GetNext(pos);
        TRACE(_T("REGENERATING %d/%d with client rect (%d, %d, %d, %d)\n"), ++i, count, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

        AppendDocument(pDC, clientRect, document, line, FALSE);

        if (concurrency::is_current_task_group_canceling())
            return;
    }

    TRACE(_T("Regenerated document to (%d, %d, %d, %d)\n"), document->dimensions.left, document->dimensions.top, document->dimensions.right, document->dimensions.bottom);
}

void CBufferCtrl::AppendDocument(CDC* pDC, const CRect clientRect, std::shared_ptr<BufferDocument> document, const CBufferLine& line, BOOL updateScrollbar)
{
    CRect timeRect(
        m_margin,
        document->dimensions.bottom,
        m_margin + m_timeWidth,
        document->dimensions.bottom + m_lineHeight);
    TextElement* timeElement = new TextElement(line.date.Format(m_timeFormat), timeRect, FALSE, FALSE);
    TRACE(_T("STRING '%s' AT (%d,%d,%d,%d)\n"), timeElement->text.GetPlain(),
        timeElement->rect.left,
        timeElement->rect.top,
        timeElement->rect.right,
        timeElement->rect.bottom);
    document->elements.AddTail(std::shared_ptr<TextElement>(timeElement));

    CRect prefixRect(
        m_margin + m_timeWidth + m_margin,
        document->dimensions.bottom,
        m_margin + m_timeWidth + m_margin + m_prefixWidth,
        document->dimensions.bottom + m_lineHeight);
    TextElement* prefixElement = new TextElement(line.prefix.GetRaw(), prefixRect, FALSE, TRUE);
    TRACE(_T("STRING '%s' AT (%d,%d,%d,%d)\n"), prefixElement->text.GetPlain(),
        prefixElement->rect.left,
        prefixElement->rect.top,
        prefixElement->rect.right,
        prefixElement->rect.bottom);
    document->elements.AddTail(std::shared_ptr<TextElement>(prefixElement));
    
    CRect messageRect(
        m_margin + m_timeWidth + m_margin + m_prefixWidth + m_margin,
        document->dimensions.bottom,
        clientRect.right - m_margin,
        document->dimensions.bottom + m_lineHeight);
    TextElement* messageElement = new TextElement(line.message.GetRaw(), messageRect, TRUE, FALSE);
    messageElement->rect = messageElement->draw(pDC, m_sds);
    TRACE(_T("STRING '%s' AT (%d,%d,%d,%d) with height %d\n"), messageElement->text.GetPlain(),
        messageElement->rect.left,
        messageElement->rect.top,
        messageElement->rect.right,
        messageElement->rect.bottom,
        messageElement->rect.Height() / m_lineHeight);
    document->elements.AddTail(std::shared_ptr<TextElement>(messageElement));

    document->dimensions |= timeElement->rect;
    document->dimensions |= prefixElement->rect;
    document->dimensions |= messageElement->rect;
}

void CBufferCtrl::RedrawVisibleElements(CDC* pDC)
{
    std::shared_ptr<BufferDocument> document = concurrency::receive(m_document);

    CDC memDC;
    memDC.CreateCompatibleDC(pDC);

    CRect rcClient;
    GetClientRect(&rcClient);

    CBitmap bitmap, *oldBitmap;
    bitmap.CreateCompatibleBitmap(pDC, rcClient.Width(), document->dimensions.Height());
    oldBitmap = memDC.SelectObject(&bitmap);
    
    int oldBkMode = memDC.SetBkMode(TRANSPARENT);

    int yOrigin;
    if (document->dimensions.Height() < rcClient.Height())
    {
        yOrigin = document->dimensions.Height() - rcClient.Height();
    }
    else
    {
        SCROLLINFO siv;
        siv.cbSize = sizeof(siv);
        GetScrollInfo(SB_VERT, &siv);
        int pageStart = GetScrollPos(SB_VERT);
        int pageEnd = pageStart + siv.nPage;
        yOrigin = pageStart;
        rcClient.OffsetRect(0, yOrigin);
    }

    memDC.FillSolidRect(rcClient, GetSysColor(COLOR_WINDOW));

    CFont font, oldFont;
    font.CreateFontIndirect(&theApp.settings.font);
    oldFont.Attach(memDC.SelectObject(font.Detach()));

    POSITION pos = document->elements.GetHeadPosition();
    while (pos != NULL)
    {
        DisplayElement& element = *document->elements.GetNext(pos);
        if (!(rcClient & element.rect).IsRectEmpty())
        {
            element.draw(&memDC, m_sds);
        }
    }

    pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, yOrigin, SRCCOPY);

    memDC.SelectObject(oldBitmap);

    font.Attach(memDC.SelectObject(oldFont.Detach()));

    memDC.SetBkMode(oldBkMode);
}

BOOL CBufferCtrl::RegisterWndClass()
{
    WNDCLASS windowclass;
    HINSTANCE hInst = AfxGetInstanceHandle();

    //Check weather the class is registerd already
    if (!(::GetClassInfo(hInst, wndClassName, &windowclass)))
    {
        //If not then we have to register the new class
        windowclass.style = CS_DBLCLKS;// | CS_HREDRAW | CS_VREDRAW;
        windowclass.lpfnWndProc = CBufferCtrl::WndProc;
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

DisplayElement::DisplayElement(const CRect rect)
    : rect(rect)
{
}

TextElement::TextElement(const CString text, const CRect rect, BOOL multiline, BOOL rightalign)
    : text(text), DisplayElement(rect), multiline(multiline), rightalign(rightalign)
{
    TRACE(_T("WIDTH: %d\n"), rect.Width());
}

CRect TextElement::draw(CDC* pDC, SCRIPT_DIGITSUBSTITUTE& sds)
{
    if (rect.Width() <= 0)
        return rect; // Degenerate case

    struct Style
    {
        LOGFONT font;
        COLORREF foreground;
        COLORREF background;
        BOOL reverseColour;
    };

    struct Colour
    {
        COLORREF colour;
        CString attrs;

        Colour(CString str, COLORREF defColour)
        {
            LPCTSTR lpsz = str;
            while (lpsz[0] ==_T('*') || lpsz[0] ==_T('!') || lpsz[0] ==_T('/') || lpsz[0] ==_T('_') || lpsz[0] ==_T('|'))
            {
                attrs += lpsz[0];
                lpsz += 1;
            }
            if (_tcslen(lpsz) == 2)
            {
                LPTSTR endptr;
                unsigned code = _tcstoul(lpsz, &endptr, 10);
                switch (code)
                {
                case 0: // Default(terminal foreground / background)
                    colour = defColour;
                    break;
                case 1: // Black
                    colour = RGB(0,0,0);
                    break;
                case 2: // Dark gray
                    colour = RGB(127,127,127);
                    break;
                case 3: // Dark red
                    colour = RGB(127,0,0);
                    break;
                case 4: // Light red
                    colour = RGB(255,0,0);
                    break;
                case 5: // Dark green
                    colour = RGB(0,147,0);
                    break;
                case 6: // Light green
                    colour = RGB(0,252,0);
                    break;
                case 7: // Brown
                    colour = RGB(252,127,0);
                    break;
                case 8: // Yellow
                    colour = RGB(255,255,0);
                    break;
                case 9: // Dark blue
                    colour = RGB(0,0,127);
                    break;
                case 10: // Light blue
                    colour = RGB(0,0,252);
                    break;
                case 11: // Dark magenta
                    colour = RGB(156,0,156);
                    break;
                case 12: // Light magenta
                    colour = RGB(255,0,255);
                    break;
                case 13: // Dark cyan
                    colour = RGB(0,147,147);
                    break;
                case 14: // Light cyan
                    colour = RGB(0,255,255);
                    break;
                case 15: // Gray
                    colour = RGB(210,210,210);
                    break;
                case 16: // White
                    colour = RGB(255,255,255);
                    break;
                default:
                    colour = defColour;
                    //{ AfxDebugBreak(); AfxThrowInvalidArgException(); }
                    break;
                }
            }
            else if (lpsz[0] == _T('@') && _tcslen(lpsz + 1) == 5)
            {
                LPTSTR endptr;
                unsigned code = _tcstoul(lpsz + 1, &endptr, 10);
                switch (code)
                {
                case 0: colour = 0x00000000; break;
                case 1: colour = 0x00800000; break;
                case 2: colour = 0x00008000; break;
                case 3: colour = 0x00808000; break;
                case 4: colour = 0x00000080; break;
                case 5: colour = 0x00800080; break;
                case 6: colour = 0x00008080; break;
                case 7: colour = 0x00c0c0c0; break;
                case 8: colour = 0x00808080; break;
                case 9: colour = 0x00ff0000; break;
                case 10: colour = 0x0000ff00; break;
                case 11: colour = 0x00ffff00; break;
                case 12: colour = 0x000000ff; break;
                case 13: colour = 0x00ff00ff; break;
                case 14: colour = 0x0000ffff; break;
                case 15: colour = 0x00ffffff; break;
                case 16: colour = 0x00000000; break;
                case 17: colour = 0x0000005f; break;
                case 18: colour = 0x00000087; break;
                case 19: colour = 0x000000af; break;
                case 20: colour = 0x000000d7; break;
                case 21: colour = 0x000000ff; break;
                case 22: colour = 0x00005f00; break;
                case 23: colour = 0x00005f5f; break;
                case 24: colour = 0x00005f87; break;
                case 25: colour = 0x00005faf; break;
                case 26: colour = 0x00005fd7; break;
                case 27: colour = 0x00005fff; break;
                case 28: colour = 0x00008700; break;
                case 29: colour = 0x0000875f; break;
                case 30: colour = 0x00008787; break;
                case 31: colour = 0x000087af; break;
                case 32: colour = 0x000087d7; break;
                case 33: colour = 0x000087ff; break;
                case 34: colour = 0x0000af00; break;
                case 35: colour = 0x0000af5f; break;
                case 36: colour = 0x0000af87; break;
                case 37: colour = 0x0000afaf; break;
                case 38: colour = 0x0000afd7; break;
                case 39: colour = 0x0000afff; break;
                case 40: colour = 0x0000d700; break;
                case 41: colour = 0x0000d75f; break;
                case 42: colour = 0x0000d787; break;
                case 43: colour = 0x0000d7af; break;
                case 44: colour = 0x0000d7d7; break;
                case 45: colour = 0x0000d7ff; break;
                case 46: colour = 0x0000ff00; break;
                case 47: colour = 0x0000ff5f; break;
                case 48: colour = 0x0000ff87; break;
                case 49: colour = 0x0000ffaf; break;
                case 50: colour = 0x0000ffd7; break;
                case 51: colour = 0x0000ffff; break;
                case 52: colour = 0x005f0000; break;
                case 53: colour = 0x005f005f; break;
                case 54: colour = 0x005f0087; break;
                case 55: colour = 0x005f00af; break;
                case 56: colour = 0x005f00d7; break;
                case 57: colour = 0x005f00ff; break;
                case 58: colour = 0x005f5f00; break;
                case 59: colour = 0x005f5f5f; break;
                case 60: colour = 0x005f5f87; break;
                case 61: colour = 0x005f5faf; break;
                case 62: colour = 0x005f5fd7; break;
                case 63: colour = 0x005f5fff; break;
                case 64: colour = 0x005f8700; break;
                case 65: colour = 0x005f875f; break;
                case 66: colour = 0x005f8787; break;
                case 67: colour = 0x005f87af; break;
                case 68: colour = 0x005f87d7; break;
                case 69: colour = 0x005f87ff; break;
                case 70: colour = 0x005faf00; break;
                case 71: colour = 0x005faf5f; break;
                case 72: colour = 0x005faf87; break;
                case 73: colour = 0x005fafaf; break;
                case 74: colour = 0x005fafd7; break;
                case 75: colour = 0x005fafff; break;
                case 76: colour = 0x005fd700; break;
                case 77: colour = 0x005fd75f; break;
                case 78: colour = 0x005fd787; break;
                case 79: colour = 0x005fd7af; break;
                case 80: colour = 0x005fd7d7; break;
                case 81: colour = 0x005fd7ff; break;
                case 82: colour = 0x005fff00; break;
                case 83: colour = 0x005fff5f; break;
                case 84: colour = 0x005fff87; break;
                case 85: colour = 0x005fffaf; break;
                case 86: colour = 0x005fffd7; break;
                case 87: colour = 0x005fffff; break;
                case 88: colour = 0x00870000; break;
                case 89: colour = 0x0087005f; break;
                case 90: colour = 0x00870087; break;
                case 91: colour = 0x008700af; break;
                case 92: colour = 0x008700d7; break;
                case 93: colour = 0x008700ff; break;
                case 94: colour = 0x00875f00; break;
                case 95: colour = 0x00875f5f; break;
                case 96: colour = 0x00875f87; break;
                case 97: colour = 0x00875faf; break;
                case 98: colour = 0x00875fd7; break;
                case 99: colour = 0x00875fff; break;
                case 100: colour = 0x00878700; break;
                case 101: colour = 0x0087875f; break;
                case 102: colour = 0x00878787; break;
                case 103: colour = 0x008787af; break;
                case 104: colour = 0x008787d7; break;
                case 105: colour = 0x008787ff; break;
                case 106: colour = 0x0087af00; break;
                case 107: colour = 0x0087af5f; break;
                case 108: colour = 0x0087af87; break;
                case 109: colour = 0x0087afaf; break;
                case 110: colour = 0x0087afd7; break;
                case 111: colour = 0x0087afff; break;
                case 112: colour = 0x0087d700; break;
                case 113: colour = 0x0087d75f; break;
                case 114: colour = 0x0087d787; break;
                case 115: colour = 0x0087d7af; break;
                case 116: colour = 0x0087d7d7; break;
                case 117: colour = 0x0087d7ff; break;
                case 118: colour = 0x0087ff00; break;
                case 119: colour = 0x0087ff5f; break;
                case 120: colour = 0x0087ff87; break;
                case 121: colour = 0x0087ffaf; break;
                case 122: colour = 0x0087ffd7; break;
                case 123: colour = 0x0087ffff; break;
                case 124: colour = 0x00af0000; break;
                case 125: colour = 0x00af005f; break;
                case 126: colour = 0x00af0087; break;
                case 127: colour = 0x00af00af; break;
                case 128: colour = 0x00af00d7; break;
                case 129: colour = 0x00af00ff; break;
                case 130: colour = 0x00af5f00; break;
                case 131: colour = 0x00af5f5f; break;
                case 132: colour = 0x00af5f87; break;
                case 133: colour = 0x00af5faf; break;
                case 134: colour = 0x00af5fd7; break;
                case 135: colour = 0x00af5fff; break;
                case 136: colour = 0x00af8700; break;
                case 137: colour = 0x00af875f; break;
                case 138: colour = 0x00af8787; break;
                case 139: colour = 0x00af87af; break;
                case 140: colour = 0x00af87d7; break;
                case 141: colour = 0x00af87ff; break;
                case 142: colour = 0x00afaf00; break;
                case 143: colour = 0x00afaf5f; break;
                case 144: colour = 0x00afaf87; break;
                case 145: colour = 0x00afafaf; break;
                case 146: colour = 0x00afafd7; break;
                case 147: colour = 0x00afafff; break;
                case 148: colour = 0x00afd700; break;
                case 149: colour = 0x00afd75f; break;
                case 150: colour = 0x00afd787; break;
                case 151: colour = 0x00afd7af; break;
                case 152: colour = 0x00afd7d7; break;
                case 153: colour = 0x00afd7ff; break;
                case 154: colour = 0x00afff00; break;
                case 155: colour = 0x00afff5f; break;
                case 156: colour = 0x00afff87; break;
                case 157: colour = 0x00afffaf; break;
                case 158: colour = 0x00afffd7; break;
                case 159: colour = 0x00afffff; break;
                case 160: colour = 0x00d70000; break;
                case 161: colour = 0x00d7005f; break;
                case 162: colour = 0x00d70087; break;
                case 163: colour = 0x00d700af; break;
                case 164: colour = 0x00d700d7; break;
                case 165: colour = 0x00d700ff; break;
                case 166: colour = 0x00d75f00; break;
                case 167: colour = 0x00d75f5f; break;
                case 168: colour = 0x00d75f87; break;
                case 169: colour = 0x00d75faf; break;
                case 170: colour = 0x00d75fd7; break;
                case 171: colour = 0x00d75fff; break;
                case 172: colour = 0x00d78700; break;
                case 173: colour = 0x00d7875f; break;
                case 174: colour = 0x00d78787; break;
                case 175: colour = 0x00d787af; break;
                case 176: colour = 0x00d787d7; break;
                case 177: colour = 0x00d787ff; break;
                case 178: colour = 0x00d7af00; break;
                case 179: colour = 0x00d7af5f; break;
                case 180: colour = 0x00d7af87; break;
                case 181: colour = 0x00d7afaf; break;
                case 182: colour = 0x00d7afd7; break;
                case 183: colour = 0x00d7afff; break;
                case 184: colour = 0x00d7d700; break;
                case 185: colour = 0x00d7d75f; break;
                case 186: colour = 0x00d7d787; break;
                case 187: colour = 0x00d7d7af; break;
                case 188: colour = 0x00d7d7d7; break;
                case 189: colour = 0x00d7d7ff; break;
                case 190: colour = 0x00d7ff00; break;
                case 191: colour = 0x00d7ff5f; break;
                case 192: colour = 0x00d7ff87; break;
                case 193: colour = 0x00d7ffaf; break;
                case 194: colour = 0x00d7ffd7; break;
                case 195: colour = 0x00d7ffff; break;
                case 196: colour = 0x00ff0000; break;
                case 197: colour = 0x00ff005f; break;
                case 198: colour = 0x00ff0087; break;
                case 199: colour = 0x00ff00af; break;
                case 200: colour = 0x00ff00d7; break;
                case 201: colour = 0x00ff00ff; break;
                case 202: colour = 0x00ff5f00; break;
                case 203: colour = 0x00ff5f5f; break;
                case 204: colour = 0x00ff5f87; break;
                case 205: colour = 0x00ff5faf; break;
                case 206: colour = 0x00ff5fd7; break;
                case 207: colour = 0x00ff5fff; break;
                case 208: colour = 0x00ff8700; break;
                case 209: colour = 0x00ff875f; break;
                case 210: colour = 0x00ff8787; break;
                case 211: colour = 0x00ff87af; break;
                case 212: colour = 0x00ff87d7; break;
                case 213: colour = 0x00ff87ff; break;
                case 214: colour = 0x00ffaf00; break;
                case 215: colour = 0x00ffaf5f; break;
                case 216: colour = 0x00ffaf87; break;
                case 217: colour = 0x00ffafaf; break;
                case 218: colour = 0x00ffafd7; break;
                case 219: colour = 0x00ffafff; break;
                case 220: colour = 0x00ffd700; break;
                case 221: colour = 0x00ffd75f; break;
                case 222: colour = 0x00ffd787; break;
                case 223: colour = 0x00ffd7af; break;
                case 224: colour = 0x00ffd7d7; break;
                case 225: colour = 0x00ffd7ff; break;
                case 226: colour = 0x00ffff00; break;
                case 227: colour = 0x00ffff5f; break;
                case 228: colour = 0x00ffff87; break;
                case 229: colour = 0x00ffffaf; break;
                case 230: colour = 0x00ffffd7; break;
                case 231: colour = 0x00ffffff; break;
                case 232: colour = 0x00080808; break;
                case 233: colour = 0x00121212; break;
                case 234: colour = 0x001c1c1c; break;
                case 235: colour = 0x00262626; break;
                case 236: colour = 0x00303030; break;
                case 237: colour = 0x003a3a3a; break;
                case 238: colour = 0x00444444; break;
                case 239: colour = 0x004e4e4e; break;
                case 240: colour = 0x00585858; break;
                case 241: colour = 0x00626262; break;
                case 242: colour = 0x006c6c6c; break;
                case 243: colour = 0x00767676; break;
                case 244: colour = 0x00808080; break;
                case 245: colour = 0x008a8a8a; break;
                case 246: colour = 0x00949494; break;
                case 247: colour = 0x009e9e9e; break;
                case 248: colour = 0x00a8a8a8; break;
                case 249: colour = 0x00b2b2b2; break;
                case 250: colour = 0x00bcbcbc; break;
                case 251: colour = 0x00c6c6c6; break;
                case 252: colour = 0x00d0d0d0; break;
                case 253: colour = 0x00dadada; break;
                case 254: colour = 0x00e4e4e4; break;
                case 255: colour = 0x00eeeeee; break;
                default: colour = 0x00777777; break;
            }
        }
        else
        {
            { AfxDebugBreak(); AfxThrowInvalidArgException(); }
        }
    }
};

LOGFONT baseFont;
pDC->GetCurrentFont()->GetLogFont(&baseFont);
COLORREF baseFgColour = pDC->GetTextColor();
COLORREF baseBgColour = pDC->GetBkColor();
Style currentStyle = { baseFont, baseFgColour, baseBgColour, FALSE };

struct Run
{
    CString text;
        SCRIPT_ANALYSIS sa;
        Style style;
    };

    CList<Run> logicalRuns;
    for (const auto& match : text)
    {
        CString szLine(match.str().c_str());
        LPCTSTR lpszLine = szLine;

        if (szLine.GetLength() <= 0)
            continue;

        switch (lpszLine[0])
        {
        case '\x19': // Colour code
        {
            CString attrs;
            switch (lpszLine[1])
            {
            case 'b': // Bar code
                break;
            case '\x1C': // Reset
                currentStyle.foreground = baseFgColour;
                currentStyle.background = baseBgColour;
                break;
            case 'F': // Foreground
            {
                Colour fg(CString(lpszLine + 2), baseFgColour);
                currentStyle.foreground = fg.colour;
                attrs += fg.attrs;
                break;
            }
            case 'B': // Background
            {
                Colour bg(CString(lpszLine + 2), baseBgColour);
                currentStyle.background = bg.colour;
                break;
            }
            case '*': // Foreground with optional background
            {
                int splitpos = CString(lpszLine + 2).Find(_T(','));
                if (splitpos >= 0)
                {
                    Colour fg(CString(lpszLine + 2, splitpos), baseFgColour);
                    currentStyle.foreground = fg.colour;
                    attrs += fg.attrs;
                    Colour bg(CString(lpszLine + 2 + splitpos + 1), baseBgColour);
                    currentStyle.background = bg.colour;
                }
                else
                {
                    Colour fg(CString(lpszLine + 2), baseFgColour);
                    currentStyle.foreground = fg.colour;
                }
                break;
            }
            case 'E': // Text emphasis
                break;
            default:
            {
                Colour fg(CString(lpszLine + 1), baseFgColour);
                currentStyle.foreground = fg.colour;
                break;
            }
            }
            for (int i = 0; i < attrs.GetLength(); i++)
            {
                switch (attrs[i])
                {
                case '*': // Bold
                    currentStyle.font.lfWeight = FW_BOLD;
                    break;
                case '!': // Reverse
                    currentStyle.reverseColour = TRUE;
                    break;
                case '/': // Italic
                    currentStyle.font.lfItalic = TRUE;
                    break;
                case '_': // Underline
                    currentStyle.font.lfUnderline = TRUE;
                    break;
                case '|': // Keep
                    break;
                default:
                    break;
                }
            }
            break;
        }
        case '\x1A': // Set attribute
            switch (lpszLine[1])
            {
            case '*': // Bold
                currentStyle.font.lfWeight = FW_BOLD;
                break;
            case '!': // Reverse
                currentStyle.reverseColour = TRUE;
                break;
            case '/': // Italic
                currentStyle.font.lfItalic = TRUE;
                break;
            case '_': // Underline
                currentStyle.font.lfUnderline = TRUE;
                break;
            case '|': // Keep
                break;
            default:
                break;
            }
            break;
        case '\x1B': // Remove attribute
            switch (lpszLine[1])
            {
            case '*': // Bold
                currentStyle.font.lfWeight = baseFont.lfWeight;
                break;
            case '!': // Reverse
                currentStyle.reverseColour = FALSE;
                break;
            case '/': // Italic
                currentStyle.font.lfItalic = baseFont.lfItalic;
                break;
            case '_': // Underline
                currentStyle.font.lfUnderline = baseFont.lfUnderline;
                break;
            case '|': // Keep
                break;
            default:
                break;
            }
            break;
        case '\x1C': // Reset
            currentStyle = { baseFont, baseFgColour, baseBgColour };
            break;
        default:
        {
            SCRIPT_CONTROL sc = { 0 };
            SCRIPT_STATE   ss = { 0 };
            VERIFY(ScriptApplyDigitSubstitution(&sds, &sc, &ss) == S_OK);
            SCRIPT_ITEM* sis;
            int itemcount = 0;
            for (int max_items = 16; ; )
            {
                sis = new SCRIPT_ITEM[max_items];
                HRESULT hr = ScriptItemize(szLine, szLine.GetLength(), max_items - 1, &sc, &ss, sis, &itemcount);

                if (SUCCEEDED(hr))
                {
                    break;
                }
                else if (hr == E_OUTOFMEMORY)
                {
                    delete[] sis;
                    max_items *= 2;
                    continue;
                }
                else
                {
                    itemcount = -1;
                    { AfxDebugBreak(); AfxThrowInvalidArgException(); }
                    break;
                }
            }
            for (int i = 0; i < itemcount; ++i)
            {
                SCRIPT_ITEM& si = sis[i];
                int startCharPos = si.iCharPos;
                int endCharPos = sis[i + 1].iCharPos;
                Run run = { CString(lpszLine + startCharPos, endCharPos - startCharPos), si.a, currentStyle };
                logicalRuns.AddTail(run);
            }
            delete[] sis;
            break;
        }
        }
    }

    CList<Run> visualRuns;
    {
        BYTE* directions = new BYTE[logicalRuns.GetSize() + 1];
        {
            int i = 0;
            for (POSITION pos = logicalRuns.GetHeadPosition(); pos != NULL; ++i)
            {
                const Run& run = logicalRuns.GetNext(pos);
                directions[i] = run.sa.s.uBidiLevel;
            }
            directions[i] = 0;
        }

        int* vtol = new int[logicalRuns.GetSize() + 1];
        int* ltov = new int[logicalRuns.GetSize() + 1];

        HRESULT hr = ScriptLayout(logicalRuns.GetSize() + 1, directions, vtol, ltov);
        for (int i = 0; i < logicalRuns.GetSize(); ++i)
        {
            const int vindex = ltov[i];
            const Run& run = logicalRuns.GetAt(logicalRuns.FindIndex(vindex));
            visualRuns.AddTail(run);
        }

        delete[] ltov;
        delete[] vtol;
        delete[] directions;
    }

    struct SubElementInfo
    {
        SCRIPT_ANALYSIS sa;
        CArray<WORD> glyphs;
        CArray<int> advances;
        CArray<GOFFSET> offsets;
        CRect rect;
        // Styles
        LOGFONT font;
        COLORREF foreground;
        COLORREF background;
    };

    CList<SubElementInfo*> subElements;
    CPoint origin = { 0, 0 };
    {
        TRACE(_T("Rect.Width: %d\n"), rect.Width());;

        SCRIPT_CACHE cache = NULL;
        for (POSITION pos = visualRuns.GetHeadPosition(); pos != NULL; )
        {
            Run& run = visualRuns.GetNext(pos);
            WORD* logs = new WORD[run.text.GetLength()];
            int max_glyphs = run.text.GetLength() * 3 / 2 + 16; // See MSDN on ScriptShape
            WORD* glyphs = new WORD[max_glyphs];
            SCRIPT_VISATTR* visattr = new SCRIPT_VISATTR[max_glyphs];

            CFont font, oldFont;
            font.CreateFontIndirect(&run.style.font);
            oldFont.Attach(pDC->SelectObject(font.Detach()));

            // TODO: Optimize
            ScriptFreeCache(&cache);
            cache = NULL;

            int glyphcount = 0;
            while (true)
            {
                HRESULT hr = ScriptShape(*pDC, &cache, run.text, run.text.GetLength(), max_glyphs, &run.sa, glyphs, logs, visattr, &glyphcount);

                if (SUCCEEDED(hr))
                {
                    // process glyphs
                    break;
                }
                else
                {
                    switch (hr)
                    {
                    case E_PENDING:
                        { AfxDebugBreak(); AfxThrowInvalidArgException(); }
                        break;
                    case E_OUTOFMEMORY:
                        max_glyphs *= 2;
                        delete[] glyphs;
                        glyphs = new WORD[max_glyphs];
                        delete[] visattr;
                        visattr = new SCRIPT_VISATTR[max_glyphs];
                        continue;
                    case USP_E_SCRIPT_NOT_IN_FONT:
                        // Try new font
                        AfxDebugBreak();
                        break;
                    default:
                        { AfxDebugBreak(); AfxThrowInvalidArgException(); }
                        break;
                    }
                }
            }

            int* advances = new int[glyphcount];
            GOFFSET* offsets = new GOFFSET[glyphcount];
            ABC abc;

            HRESULT hr = ScriptPlace(*pDC, &cache, glyphs, glyphcount, visattr, &run.sa, advances, offsets, &abc);

            if (FAILED(hr))
                { AfxDebugBreak(); AfxThrowInvalidArgException(); }

            SCRIPT_LOGATTR* logattr = new SCRIPT_LOGATTR[run.text.GetLength()];

            hr = ScriptBreak(run.text, run.text.GetLength(), &run.sa, logattr);

            if (FAILED(hr))
                { AfxDebugBreak(); AfxThrowInvalidArgException(); }

            long fontHeight;

            hr = ScriptCacheGetHeight(*pDC, &cache, &fontHeight);

            if (FAILED(hr))
                { AfxDebugBreak(); AfxThrowInvalidArgException(); }

            int* alpdx = new int[run.text.GetLength()];
            hr = ScriptGetLogicalWidths(&run.sa, run.text.GetLength(), glyphcount, advances, logs, visattr, alpdx);

            if (FAILED(hr))
                { AfxDebugBreak(); AfxThrowInvalidArgException(); }

            TRACE(_T("Run text: %s\n"), run.text);

            for (int lineStartCh = 0; ; )
            {
                TRACE(_T("Getting Text Extent: '%s' in %d\n"), CString(run.text.GetString() + lineStartCh, run.text.GetLength() - lineStartCh), rect.Width() - origin.x);

                int nChFit = 0;
                for (int lineWidth = 0; nChFit < run.text.GetLength() - lineStartCh && lineWidth + alpdx[nChFit + lineStartCh] <= rect.Width() - origin.x; ++nChFit)
                {
                    lineWidth += alpdx[nChFit + lineStartCh];
                    LPCTSTR lineChars = run.text.GetString() + lineStartCh;
                    TRACE(_T(": '%c'@%d (+%d) on (%d/%d)\n"), lineChars[nChFit], nChFit, alpdx[nChFit + lineStartCh], lineWidth, rect.Width() - origin.x);
                }
                
                int lineEndCh = -1;
                if (multiline && nChFit < run.text.GetLength() - lineStartCh)
                { // Break line
                    for (int n = nChFit; n >= 0; --n)
                    { // Find last soft break
                        SCRIPT_LOGATTR& la = logattr[n + lineStartCh];
                        if ((n > 0 && la.fSoftBreak) || (n == 0 && origin.x > 0))
                        { // Soft break available at n
                            TRACE(_T("(%d, %d) Soft breaking '%s' at '%s'\n"), origin.x, origin.y, run.text, CString(run.text.GetString() + lineStartCh, n));
                            lineEndCh = n + lineStartCh;
                            break;
                        }
                    }
                    if (lineEndCh < 0)
                    { // No soft break found in current run
                        for (int n = nChFit - 1; n > 0; --n)
                        { // Find last char stop
                            SCRIPT_LOGATTR& la = logattr[n + lineStartCh];
                            if (la.fCharStop)
                            { // Hard break available at n
                                TRACE(_T("(%d, %d) Hard breaking '%s' at '%s'\n"), origin.x, origin.y, run.text, CString(run.text.GetString() + lineStartCh, n));
                                lineEndCh = n + lineStartCh;
                                break;
                            }
                        }
                        if (lineEndCh < 0)
                        { // No char stop found
                            for (int n = 1; n < run.text.GetLength() - lineStartCh; ++n)
                            { // Find first char stop
                                SCRIPT_LOGATTR& la = logattr[n + lineStartCh];
                                if (la.fCharStop)
                                { // Violent break available at n
                                    TRACE(_T("(%d, %d) Violently breaking '%s' at '%s'\n"), origin.x, origin.y, run.text, CString(run.text.GetString() + lineStartCh, n));
                                    lineEndCh = n + lineStartCh;
                                    break;
                                }
                            }
                            if (lineEndCh < 0)
                            { // No char stops in this cluster
                                TRACE(_T("(%d, %d) No valid breakpoints in line (%d:%d)\n"), origin.x, origin.y, lineStartCh, run.text.GetLength() - lineStartCh);
                                lineEndCh = run.text.GetLength();
                            }
                        }
                    }
                }
                else
                { // No need to break line
                    TRACE(_T("(%d, %d) Not breaking line '%s' (%s)\n"), origin.x, origin.y, run.text, CString(run.text.GetString() + lineStartCh, nChFit));
                    lineEndCh = run.text.GetLength();
                }

                if (lineEndCh > lineStartCh)
                {
                    SubElementInfo* info = new SubElementInfo;
                    info->sa = run.sa;
                    info->glyphs.SetSize(logs[lineEndCh - 1] - logs[lineStartCh] + 1);
                    for (int j = 0; j < info->glyphs.GetSize(); ++j)
                        info->glyphs.SetAt(j, glyphs[j + logs[lineStartCh]]);
                    info->advances.SetSize(logs[lineEndCh - 1] - logs[lineStartCh] + 1);
                    for (int j = 0; j < info->advances.GetSize(); ++j)
                        info->advances.SetAt(j, advances[j + logs[lineStartCh]]);
                    info->offsets.SetSize(logs[lineEndCh - 1] - logs[lineStartCh] + 1);
                    for (int j = 0; j < info->offsets.GetSize(); ++j)
                        info->offsets.SetAt(j, offsets[j + logs[lineStartCh]]);
                    info->rect.left = origin.x;
                    info->rect.top = origin.y;
                    info->rect.right = origin.x;
                    for (int j = lineStartCh; j < lineEndCh; ++j)
                        info->rect.right += alpdx[j];
                    info->rect.bottom = origin.y + fontHeight;
                    info->font = run.style.font;
                    info->foreground = run.style.reverseColour ? run.style.background : run.style.foreground;
                    info->background = run.style.reverseColour ? run.style.foreground : run.style.background;
                    if (multiline)
                        ASSERT(info->rect.right <= rect.right);
                    subElements.AddTail(info);
                }

                TRACE(_T("LEC, RTL = %d, %d\n"), lineEndCh, run.text.GetLength());
                if (multiline && lineEndCh < run.text.GetLength())
                { // Line broken
                    origin.x = 0; // CR
                    origin.y += fontHeight; // LF
                    lineStartCh = lineEndCh;
                    continue;
                }
                else
                { // Line unfinished
                    for (int j = lineStartCh; j < run.text.GetLength(); ++j)
                        origin.x += alpdx[j];
                    break;
                }
            }

            delete[] alpdx;
            delete[] logattr;
            delete[] advances;
            delete[] offsets;
            delete[] logs;
            delete[] glyphs;
            delete[] visattr;

            font.Attach(pDC->SelectObject(oldFont.Detach()));
        }
        ScriptFreeCache(&cache);
    }

    CRect fullRect = rect;
    if (multiline)
        fullRect.bottom = fullRect.top + 1;
    else
        fullRect.right = fullRect.left + 1;
    for (POSITION pos = subElements.GetHeadPosition();  pos != NULL; )
    {
        const SubElementInfo* info = subElements.GetNext(pos);
        SCRIPT_CACHE cache = NULL;
        CRect subRect = info->rect;
        subRect.OffsetRect(fullRect.left, fullRect.top);
        if (!multiline && rightalign)
            subRect.OffsetRect(rect.Width() - origin.x, 0);
        CString text((wchar_t*)info->glyphs.GetData(), info->glyphs.GetSize());
        TRACE(_T("Drawing '%s' to (%d, %d, %d, %d)\n"), text, subRect.left, subRect.top, subRect.right, subRect.bottom);

        pDC->SetTextColor(info->foreground);
        pDC->SetBkColor(info->background);

        HRESULT hr = ScriptTextOut(*pDC, &cache, subRect.left, subRect.top, 0, subRect, &info->sa, NULL, 0,
            info->glyphs.GetData(), info->glyphs.GetSize(), info->advances.GetData(), NULL, info->offsets.GetData());

        if (FAILED(hr))
            { AfxDebugBreak(); AfxThrowInvalidArgException(); }

        fullRect |= subRect;
        delete info;
    }

    pDC->SetTextColor(baseFgColour);
    pDC->SetBkColor(baseBgColour);

    TRACE(_T("Height = %d\n"), fullRect.Height());
    return fullRect;
}
