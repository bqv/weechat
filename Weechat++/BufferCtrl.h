#pragma once


// CBufferCtrl

class CBufferCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CBufferCtrl)

public:
	CBufferCtrl();
	virtual ~CBufferCtrl();

protected:
	DECLARE_MESSAGE_MAP()

private:
    BOOL RegisterWndClass();
};


