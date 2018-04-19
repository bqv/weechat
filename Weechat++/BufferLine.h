#pragma once

// CBufferLine command target

struct CBufferLine : CObject
{
	CBufferLine();
	CBufferLine(const CBufferLine& other);
	virtual ~CBufferLine();
    CBufferLine& operator=(const CBufferLine& other);

// Attributes
    CString buffer;
    CTime date;
    CTime datePrinted;
    BOOL displayed;
    BOOL highlight;
    CList<CString> tagsArray;
    CString prefix;
    CString message;
};


