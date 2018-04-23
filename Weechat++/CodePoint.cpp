#include "stdafx.h"
#include "CodePoint.h"

CStringA UTF16toUTF8(const CStringW& utf16)
{
    LPSTR pszUtf8 = NULL;
    CStringA utf8("");

    if (utf16.IsEmpty())
        return utf8; //empty imput string

    size_t nLen16 = utf16.GetLength();
    size_t nLen8 = 0;

    if ((nLen8 = WideCharToMultiByte(CP_UTF8, 0, utf16, nLen16,
        NULL, 0, 0, 0) + 2) == 2)
        return utf8; //conversion error!

    pszUtf8 = new char[nLen8];
    if (pszUtf8)
    {
        memset(pszUtf8, 0x00, nLen8);
        WideCharToMultiByte(CP_UTF8, 0, utf16, nLen16, pszUtf8, nLen8, 0, 0);
        utf8 = CStringA(pszUtf8, nLen8);
    }

    delete[] pszUtf8;
    return utf8; //utf8 encoded string
}

CStringW UTF8toUTF16(const CStringA& utf8)
{
    LPWSTR pszUtf16 = NULL;
    CStringW utf16("");

    if (utf8.IsEmpty())
        return utf16; //empty imput string

    size_t nLen8 = utf8.GetLength();
    size_t nLen16 = 0;

    if ((nLen16 = MultiByteToWideChar(CP_UTF8, 0, utf8, nLen8, NULL, 0)) == 0)
        return utf16; //conversion error!

    pszUtf16 = new wchar_t[nLen16];
    if (pszUtf16)
    {
        wmemset(pszUtf16, 0x00, nLen16);
        MultiByteToWideChar(CP_UTF8, 0, utf8, nLen8, pszUtf16, nLen16);
        utf16 = CStringW(pszUtf16, nLen16);
    }

    delete[] pszUtf16;
    return utf16; //utf16 encoded string
}
