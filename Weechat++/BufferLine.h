#pragma once

#include "WeechatString.h"

// CBufferLine command target

struct CBufferLine : CObject
{
	CBufferLine();
	CBufferLine(const CBufferLine& other);
	virtual ~CBufferLine();
    CBufferLine& operator=(const CBufferLine& other);

// Attributes
    CString pointer;
    CString buffer;
    CTime date;
    CTime datePrinted;
    BOOL displayed;
    BOOL highlight;
    CList<CString> tagsArray;
    CWeechatString prefix;
    CWeechatString message;
};


