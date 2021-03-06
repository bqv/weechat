
// WeechatBuffer.cpp : implementation of the CWeechatBuffer class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Main.h"
#endif

#include "WeechatBuffer.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CWeechatBuffer

IMPLEMENT_DYNCREATE(CWeechatBuffer, CDocument)

BEGIN_MESSAGE_MAP(CWeechatBuffer, CDocument)
END_MESSAGE_MAP()


// CWeechatBuffer construction/destruction

CWeechatBuffer::CWeechatBuffer()
{
	// TODO: add one-time construction code here
    
}

CWeechatBuffer::CWeechatBuffer(const CWeechatBuffer & other)
{
    m_pointer = other.m_pointer;
    m_number = other.m_number;
    m_name = other.m_name;
    m_fullName = other.m_fullName;
    m_shortName = other.m_shortName;
    m_hasNicklist = other.m_hasNicklist;
    m_title = other.m_title;
    m_localVariables.RemoveAll();
    POSITION pos = other.m_localVariables.GetStartPosition();
    while (pos != NULL)
    {
        CString key;
        CString value;

        other.m_localVariables.GetNextAssoc(pos, key, value);
        m_localVariables.SetAt(key, value);
    }
    m_prevBuffer = other.m_prevBuffer;
    m_nextBuffer = other.m_nextBuffer;
    m_type = other.m_type;
}

CWeechatBuffer CWeechatBuffer::operator=(const CWeechatBuffer & other)
{
    if (this != &other)
    {
        m_pointer = other.m_pointer;
        m_number = other.m_number;
        m_name = other.m_name;
        m_fullName = other.m_fullName;
        m_shortName = other.m_shortName;
        m_hasNicklist = other.m_hasNicklist;
        m_title = other.m_title;
        m_localVariables.RemoveAll();
        POSITION pos = other.m_localVariables.GetStartPosition();
        while (pos != NULL)
        {
            CString key;
            CString value;

            other.m_localVariables.GetNextAssoc(pos, key, value);
            m_localVariables.SetAt(key, value);
        }
        m_prevBuffer = other.m_prevBuffer;
        m_nextBuffer = other.m_nextBuffer;
        m_type = other.m_type;
    }
    return *this;
}

CWeechatBuffer::~CWeechatBuffer()
{
}

void CWeechatBuffer::AddLine(const CBufferLine & line)
{
}

BOOL CWeechatBuffer::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// CWeechatBuffer serialization

void CWeechatBuffer::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CWeechatBuffer::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CWeechatBuffer::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CWeechatBuffer::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CWeechatBuffer diagnostics

#ifdef _DEBUG
void CWeechatBuffer::AssertValid() const
{
	CDocument::AssertValid();
}

void CWeechatBuffer::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CWeechatBuffer commands
