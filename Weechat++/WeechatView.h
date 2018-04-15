
// WeechatView.h : interface of the CWeechatView class
//

#pragma once

#include "WndResizer.h"
#include "BufferCtrl.h"

class CWeechatView : public CFormView
{
protected: // create from serialization only
	CWeechatView();
	DECLARE_DYNCREATE(CWeechatView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_FORM };
#else
    enum { IDD = IDD_WEECHAT_FORM };
#endif

// Attributes
public:
	CWeechatDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

// Implementation
public:
	virtual ~CWeechatView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
private:
    CBufferCtrl m_bufferCtrl;
    CWndResizer m_resizer;
    CSize m_szDesignTimeSize;

protected:

// Generated message map functions
protected:
    afx_msg void OnFilePrintPreview();
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WeechatView.cpp
inline CWeechatDoc* CWeechatView::GetDocument() const
   { return reinterpret_cast<CWeechatDoc*>(m_pDocument); }
#endif

