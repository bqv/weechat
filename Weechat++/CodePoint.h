#pragma once

#ifdef _UNICODE
#define UTF8TOCS(x) UTF8toUTF16(CStringA(x))
#define CSTOUTF8(x) UTF16toUTF8(x).GetString()
#else
#define UTF8TOCS(x) CStringA(x)
#define CSTOUTF8(x) x.GetString()
#endif

CStringA UTF16toUTF8(const CStringW& utf16);
CStringW UTF8toUTF16(const CStringA& utf8);
