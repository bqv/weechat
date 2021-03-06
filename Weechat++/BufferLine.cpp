// BufferLine.cpp : implementation file
//

#include "stdafx.h"
#include "BufferLine.h"

// CBufferLine

CBufferLine::CBufferLine()
{
}

CBufferLine::CBufferLine(const CBufferLine & other)
{
    pointer = other.pointer;
    buffer = other.buffer;
    date = other.date;
    datePrinted = other.datePrinted;
    displayed = other.displayed;
    highlight = other.highlight;
    tagsArray.RemoveAll();
    for (POSITION pos = other.tagsArray.GetHeadPosition(); pos; )
    {
        const auto& item = other.tagsArray.GetNext(pos);
        tagsArray.AddTail(item);
    }
    prefix = other.prefix;
    message = other.message;
}

CBufferLine::~CBufferLine()
{
}

CBufferLine& CBufferLine::operator=(const CBufferLine& other)
{
    if (&other == this)
        return *this;

    pointer = other.pointer;
    buffer = other.buffer;
    date = other.date;
    datePrinted = other.datePrinted;
    displayed = other.displayed;
    highlight = other.highlight;
    tagsArray.RemoveAll();
    for (POSITION pos = other.tagsArray.GetHeadPosition(); pos; )
    {
        const auto& item = other.tagsArray.GetNext(pos);
        tagsArray.AddTail(item);
    }
    prefix = other.prefix;
    message = other.message;

    return *this;
}

// CBufferLine member functions
