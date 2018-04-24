// BufferCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BufferCtrl.h"

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
    ON_WM_SIZE()
    ON_WM_VSCROLL()
    ON_WM_SETTINGCHANGE()
    ON_WM_NCDESTROY()
END_MESSAGE_MAP()

void CBufferCtrl::SetLines(CList<CBufferLine>& lines)
{
    CDC memDC;
    memDC.CreateCompatibleDC(NULL);

    m_lines.RemoveAll();
    for (POSITION pos = lines.GetHeadPosition(); pos != NULL; )
    {
        CBufferLine& line = lines.GetNext(pos);
        if (::IsWindow(m_hWnd))
        {
            SIZE size;

            CString timeStr = line.date.Format(m_timeFormat);
            GetTextExtentPoint(memDC, timeStr, timeStr.GetLength(), &size);
            m_timeWidth = max(m_timeWidth, size.cx);

            CString prefixStr = line.prefix.GetPlain() + " ";
            GetTextExtentPoint(memDC, prefixStr, prefixStr.GetLength(), &size);
            m_prefixWidth = max(m_prefixWidth, size.cx);

            m_lineHeight = size.cy;
        }
        m_lines.AddTail(line);
    }
    RegenerateDocument(&memDC);
}

void CBufferCtrl::AddLine(CBufferLine& line)
{
    CDC memDC;
    memDC.CreateCompatibleDC(NULL);
    SIZE size;

    CString timeStr = line.date.Format(m_timeFormat);
    GetTextExtentPoint(memDC, timeStr, timeStr.GetLength(), &size);
    m_timeWidth = max(m_timeWidth, size.cx);

    CString prefixStr = line.prefix.GetPlain();
    GetTextExtentPoint(memDC, prefixStr, prefixStr.GetLength(), &size);
    m_prefixWidth = max(m_prefixWidth, size.cx);

    m_lineHeight = size.cy;
    m_lines.AddTail(line);
    AppendDocument(&memDC, line);
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
    VERIFY(m_document.font.CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE,
        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN,
        _T("Courier New")));

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
    CRect rcClient;
    GetClientRect(rcClient);

    if (m_document.dimensions.Height() < rcClient.Height())
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
        OnVScroll(SB_LINEDOWN, abs(zDelta), NULL);
    else
        OnVScroll(SB_LINEUP, abs(zDelta), NULL);

    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CBufferCtrl::OnSize(UINT nType, int cx, int cy)
{
    CDC memDC;
    memDC.CreateCompatibleDC(NULL);
    RegenerateDocument(&memDC);
    CWnd::OnSize(nType, cx, cy);
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
    int iOldPos = curpos;

    // Determine the new position of scroll box.
    switch (nSBCode)
    {
    case SB_LEFT:      // Scroll to far left.
        curpos = minpos;
        break;

    case SB_RIGHT:      // Scroll to far right.
        curpos = maxpos;
        break;

    case SB_ENDSCROLL:   // End scroll.
        break;

    case SB_LINELEFT:      // Scroll left.
        if (curpos > minpos)
            curpos--;
        break;

    case SB_LINERIGHT:   // Scroll right.
        if (curpos < maxpos)
            curpos++;
        break;

    case SB_PAGELEFT:    // Scroll one page left.
    {
        // Get the page size. 
        SCROLLINFO info;
        GetScrollInfo(SB_VERT, &info, SIF_ALL);

        if (curpos > minpos)
            curpos = max(minpos, curpos - (int)info.nPage);
        break;
    }

    case SB_PAGERIGHT:      // Scroll one page right.
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

    int iScrolled = iOldPos - curpos;

    if (iScrolled != 0)
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        
        ScrollWindowEx(0, m_lineHeight * iScrolled, rcClient, NULL, NULL, NULL, SW_SMOOTHSCROLL | SW_ERASE);

        m_follow = (curpos == maxpos);

        Invalidate(FALSE);
        UpdateWindow();
    }

    SetScrollPos(SB_VERT, curpos);
    CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CBufferCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    VERIFY(ScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT, &m_sds) == S_OK);
}

void CBufferCtrl::OnNcDestroy()
{
    delete this;
}

void CBufferCtrl::RegenerateDocument(CDC* pDC)
{
    CRect clientRect = {0,0,GetDeviceCaps(*pDC, HORZRES),GetDeviceCaps(*pDC, VERTRES)};
    if (::IsWindow(m_hWnd))
    {
        GetClientRect(&clientRect);

        m_document.elements.RemoveAll();
        m_document.dimensions.SetRectEmpty();

        POSITION pos = m_lines.GetHeadPosition();
        const int count = m_lines.GetSize();
        int i = 0;
        while (pos != NULL)
        {
            const CBufferLine& line = m_lines.GetNext(pos);
            TRACE(_T("REGENERATING %d/%d\n"), ++i, count);

            AppendDocument(pDC, line);
        }

        int documentLines = m_document.dimensions.Height() / m_lineHeight;
        SCROLLINFO siv;
        siv.cbSize = sizeof(SCROLLINFO);
        GetScrollInfo(SB_VERT, &siv);
        siv.nMin = 0;
        siv.nMax = documentLines;
        siv.nPage = clientRect.Height() / m_lineHeight;
        SetScrollInfo(SB_VERT, &siv);
    }
}

void CBufferCtrl::AppendDocument(CDC* pDC, const CBufferLine& line)
{
    CRect clientRect = {0,0,GetDeviceCaps(*pDC, HORZRES),GetDeviceCaps(*pDC, VERTRES)};
    if (::IsWindow(m_hWnd))
    {
        GetClientRect(&clientRect);

        CRect timeRect(
            m_margin,
            m_document.dimensions.bottom,
            m_margin + m_timeWidth,
            m_document.dimensions.bottom + m_lineHeight);
        TextElement* timeElement = new TextElement(line.date.Format(m_timeFormat), timeRect, FALSE);
        TRACE(_T("STRING '%s' AT (%d,%d,%d,%d)\n"), timeElement->text.GetPlain(),
            timeElement->rect.left,
            timeElement->rect.top,
            timeElement->rect.right,
            timeElement->rect.bottom);
        m_document.elements.AddTail(std::shared_ptr<TextElement>(timeElement));

        CRect prefixRect(
            m_margin + m_timeWidth + m_margin,
            m_document.dimensions.bottom,
            m_margin + m_timeWidth + m_margin + m_prefixWidth,
            m_document.dimensions.bottom + m_lineHeight);
        TextElement* prefixElement = new TextElement(line.prefix.GetRaw(), prefixRect, FALSE);
        TRACE(_T("STRING '%s' AT (%d,%d,%d,%d)\n"), prefixElement->text.GetPlain(),
            prefixElement->rect.left,
            prefixElement->rect.top,
            prefixElement->rect.right,
            prefixElement->rect.bottom);
        m_document.elements.AddTail(std::shared_ptr<TextElement>(prefixElement));

        CRect messageRect(
            m_margin + m_timeWidth + m_margin + m_prefixWidth + m_margin,
            m_document.dimensions.bottom,
            clientRect.right - m_margin,
            m_document.dimensions.bottom + m_lineHeight);
        TextElement* messageElement = new TextElement(line.message.GetRaw(), messageRect, TRUE);
        messageElement->rect = messageElement->draw(pDC, m_sds);
        TRACE(_T("STRING '%s' AT (%d,%d,%d,%d)\n"), messageElement->text.GetPlain(),
            messageElement->rect.left,
            messageElement->rect.top,
            messageElement->rect.right,
            messageElement->rect.bottom);
        m_document.elements.AddTail(std::shared_ptr<TextElement>(messageElement));
        m_document.dimensions |= messageElement->rect;
    }
}

void CBufferCtrl::RedrawVisibleElements(CDC* pDC)
{
    CDC memDC;
    memDC.CreateCompatibleDC(pDC);

    CRect rcClient;
    GetClientRect(&rcClient);

    CBitmap bitmap, *oldBitmap;
    bitmap.CreateCompatibleBitmap(pDC, rcClient.Width(), m_document.dimensions.Height());
    oldBitmap = memDC.SelectObject(&bitmap);

    int yOrigin;
    if (m_document.dimensions.Height() < rcClient.Height())
    {
        yOrigin = m_document.dimensions.Height() - rcClient.Height();
    }
    else
    {
        SCROLLINFO siv;
        siv.cbSize = sizeof(siv);
        GetScrollInfo(SB_VERT, &siv);
        int pageStart = GetScrollPos(SB_VERT);
        int pageEnd = pageStart + siv.nPage;
        yOrigin = pageStart * m_lineHeight;
        rcClient.OffsetRect(0, yOrigin);
    }

    memDC.FillSolidRect(rcClient, GetSysColor(COLOR_WINDOW));

    CFont oldFont;
    oldFont.Attach(memDC.SelectObject(m_document.font.Detach()));

    POSITION pos = m_document.elements.GetHeadPosition();
    while (pos != NULL)
    {
        DisplayElement& element = *m_document.elements.GetNext(pos);
        if (!(rcClient & element.rect).IsRectEmpty())
            element.draw(&memDC, m_sds);
    }

    pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, yOrigin, SRCCOPY);

    memDC.SelectObject(oldBitmap);

    m_document.font.Attach(memDC.SelectObject(oldFont.Detach()));
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

TextElement::TextElement(const CString text, const CRect rect, BOOL multiline)
    : text(text), DisplayElement(rect)
{
    if (!multiline)
    {
        multiline = false;
    }
    else
    {
        multiline = true;
    }
}

CRect TextElement::draw(CDC* pDC, SCRIPT_DIGITSUBSTITUTE& sds)
{
    if (rect.Width() < 0)
        return rect; // Degenerate case

    struct Run
    {
        CString text;
        SCRIPT_ANALYSIS sa;
        LOGFONT font;
        COLORREF foreground;
        COLORREF background;
    };

    LOGFONT currentFont;
    pDC->GetCurrentFont()->GetLogFont(&currentFont);
    COLORREF currFgColour = pDC->GetTextColor();
    COLORREF currBgColour = pDC->GetBkColor();

    CList<Run> logicalRuns;
    for (const auto& match : text)
    {
        CString szLine(match.str().c_str());
        LPCTSTR lpszLine = szLine;

        if (szLine.GetLength() <= 0)
            continue;

        switch (lpszLine[0])
        {
        case '\x19':
            break;
        case '\x1A':
            break;
        case '\x1B':
            break;
        case '\x1C':
            break;
        default:
        {
            SCRIPT_CONTROL sc = { 0 };
            SCRIPT_STATE   ss = { 0 };
            VERIFY(ScriptApplyDigitSubstitution(&sds, &sc, &ss) == S_OK);
            int max_items = 16;
            SCRIPT_ITEM* sis = new SCRIPT_ITEM[max_items];
            int itemcount = 0;
            while (true)
            {
                HRESULT hr = ScriptItemize(szLine, szLine.GetLength(), max_items - 1, &sc, &ss, sis, &itemcount);

                if (SUCCEEDED(hr))
                {
                    break;
                }
                else if (hr == E_OUTOFMEMORY)
                {
                    delete[] sis;
                    sis = new SCRIPT_ITEM[max_items *= 2];
                    continue;
                }
                else
                {
                    itemcount = -1;
                    AfxThrowInvalidArgException();
                    break;
                }
            }
            for (int i = 0; i < itemcount; ++i)
            {
                SCRIPT_ITEM& si = sis[i];
                int startCharPos = si.iCharPos;
                int endCharPos = sis[i + 1].iCharPos;
                Run run = { CString(lpszLine + startCharPos, endCharPos - startCharPos), si.a, currentFont, currFgColour, currBgColour };
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
        CString text;
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
        SCRIPT_CACHE cache = NULL;
        for (POSITION pos = visualRuns.GetHeadPosition(); pos != NULL; )
        {
            Run& run = visualRuns.GetNext(pos);
            WORD* logs = new WORD[run.text.GetLength()];
            int max_glyphs = run.text.GetLength() * 3 / 2 + 16; // See MSDN on ScriptShape
            WORD* glyphs = new WORD[max_glyphs];
            SCRIPT_VISATTR* visattr = new SCRIPT_VISATTR[max_glyphs];

            CFont font, oldFont;
            font.CreateFontIndirect(&run.font);
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
                        AfxThrowInvalidArgException();
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
                        AfxThrowInvalidArgException();
                        break;
                    }
                }
            }

            int* advances = new int[glyphcount];
            GOFFSET* offsets = new GOFFSET[glyphcount];
            ABC abc;

            HRESULT hr = ScriptPlace(*pDC, &cache, glyphs, glyphcount, visattr, &run.sa, advances, offsets, &abc);

            if (FAILED(hr))
                AfxThrowInvalidArgException();

            SCRIPT_LOGATTR* logattr = new SCRIPT_LOGATTR[run.text.GetLength()];

            hr = ScriptBreak(run.text, run.text.GetLength(), &run.sa, logattr);

            if (FAILED(hr))
                AfxThrowInvalidArgException();

            long fontHeight;
            hr = ScriptCacheGetHeight(*pDC, &cache, &fontHeight);

            if (FAILED(hr))
                AfxThrowInvalidArgException();

            int vpos = origin.x, vposLastBreak = origin.x;
            int lastBreak = 0, lineStart = 0;
            int lastChar = 0, vposLastChar = origin.x;
            LPCTSTR lpszText = run.text;
            for (int i = 0; i < run.text.GetLength(); ++i)
            {
                SCRIPT_LOGATTR& la = logattr[i];
                if (la.fSoftBreak)
                {
                    lastBreak = i;
                    vposLastBreak = vpos;
                    lastChar = i;
                    vposLastChar = vpos;
                }
                else if (la.fCharStop)
                {
                    lastChar = i;
                    vposLastChar = vpos;
                }

                if (multiline && vpos + advances[i] > rect.Width())
                {
                    if (lastBreak == lineStart)
                    { // Break line rudely
                        if (lastChar == lineStart)
                        {
                            // Force draw one character
                            vpos += advances[i];
                        }
                        else
                        {
                            SubElementInfo* info = new SubElementInfo;
                            info->text = CString(lpszText + lineStart, lastChar - lineStart);
                            info->sa = run.sa;
                            info->glyphs.SetSize(logs[lastChar] - logs[lineStart] + 1);
                            for (int j = logs[lineStart]; j <= logs[lastChar]; ++j)
                                info->glyphs.SetAt(j - logs[lineStart], glyphs[j]);
                            info->advances.SetSize(logs[lastChar] - logs[lineStart] + 1);
                            for (int j = logs[lineStart]; j <= logs[lastChar]; ++j)
                                info->advances.SetAt(j - logs[lineStart], advances[j]);
                            info->offsets.SetSize(logs[lastChar] - logs[lineStart] + 1);
                            for (int j = logs[lineStart]; j <= logs[lastChar]; ++j)
                                info->offsets.SetAt(j - logs[lineStart], offsets[j]);
                            info->rect.left = origin.x;
                            info->rect.top = origin.y;
                            info->rect.right = vposLastBreak;
                            info->rect.bottom = origin.y + fontHeight;
                            info->font = run.font;
                            info->foreground = run.foreground;
                            info->background = run.background;
                            subElements.AddTail(info);

                            origin.x = 0;
                            origin.y += fontHeight;
                            vpos = 0;
                            lineStart = lastChar;
                        }
                    }
                    else
                    { // Break line at lastBreak
                        SubElementInfo* info = new SubElementInfo;
                        info->text = CString(lpszText + lineStart, lastBreak - lineStart);
                        info->sa = run.sa;
                        info->glyphs.SetSize(logs[lastBreak] - logs[lineStart] + 1);
                        for (int j = logs[lineStart]; j <= logs[lastBreak]; ++j)
                            info->glyphs.SetAt(j - logs[lineStart], glyphs[j]);
                        wchar_t* data = (wchar_t*)info->glyphs.GetData();
                        TRACE(_T("!! GLYPHS: '%s' (of '%s')\n"), CString(data, logs[lastBreak] - logs[lineStart] + 1), CString((wchar_t*)glyphs, glyphcount));
                        info->advances.SetSize(logs[lastBreak] - logs[lineStart] + 1);
                        for (int j = logs[lineStart]; j <= logs[lastBreak]; ++j)
                            info->advances.SetAt(j - logs[lineStart], advances[j]);
                        info->offsets.SetSize(logs[lastBreak] - logs[lineStart] + 1);
                        for (int j = logs[lineStart]; j <= logs[lastBreak]; ++j)
                            info->offsets.SetAt(j - logs[lineStart], offsets[j]);
                        info->rect.left = origin.x;
                        info->rect.top = origin.y;
                        info->rect.right = vposLastBreak;
                        info->rect.bottom = origin.y + fontHeight;
                        info->font = run.font;
                        info->foreground = run.foreground;
                        info->background = run.background;
                        subElements.AddTail(info);

                        origin.x = 0;
                        origin.y += fontHeight;
                        vpos = 0;
                        lineStart = lastBreak;
                    }
                }
                else
                {
                    vpos += advances[i];
                }
            }
            SubElementInfo* info = new SubElementInfo;
            info->text = CString(lpszText + lineStart, run.text.GetLength() - lineStart);
            info->sa = run.sa;
            info->glyphs.SetSize(logs[run.text.GetLength() - 1] - logs[lineStart] + 1);
            for (int j = logs[lineStart]; j <= logs[run.text.GetLength() - 1]; ++j)
                info->glyphs.SetAt(j - logs[lineStart], glyphs[j]);
            info->advances.SetSize(logs[run.text.GetLength() - 1] - logs[lineStart] + 1);
            for (int j = logs[lineStart]; j <= logs[run.text.GetLength() - 1]; ++j)
                info->advances.SetAt(j - logs[lineStart], advances[j]);
            info->offsets.SetSize(logs[run.text.GetLength() - 1] - logs[lineStart] + 1);
            for (int j = logs[lineStart]; j <= logs[run.text.GetLength() - 1]; ++j)
                info->offsets.SetAt(j - logs[lineStart], offsets[j]);
            info->rect.left = origin.x;
            info->rect.top = origin.y;
            info->rect.right = vposLastBreak;
            info->rect.bottom = origin.y + fontHeight;
            info->font = run.font;
            info->foreground = run.foreground;
            info->background = run.background;
            subElements.AddTail(info);
            origin.x = vpos;

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
    POSITION pos = subElements.GetHeadPosition();
    while (pos != NULL)
    {
        const SubElementInfo* info = subElements.GetNext(pos);
        SCRIPT_CACHE cache = NULL;
        CRect subRect = info->rect;
        subRect.OffsetRect(fullRect.left, fullRect.top);
        TRACE(_T("Drawing '%s' to (%d, %d, %d, %d)\n"), info->text, subRect.left, subRect.top, subRect.right, subRect.bottom);
        HRESULT hr;
        hr = ScriptTextOut(*pDC, &cache, subRect.left, subRect.top, 0, subRect, &info->sa, NULL, 0,
            info->glyphs.GetData(), info->glyphs.GetSize(), info->advances.GetData(), NULL, info->offsets.GetData());
        fullRect |= subRect;
        delete info;
    }
    return fullRect;
}
