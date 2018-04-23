#include "stdafx.h"
#include "WeechatString.h"

#include <sstream>
#include <algorithm>

#ifdef _UNICODE
const std::wstring CWeechatString::RE_COLOR_ATTRS(L"[*!/_|]*");
const std::wstring CWeechatString::RE_COLOR_STD(L"(?:" + RE_COLOR_ATTRS + L"\\d{2})");
const std::wstring CWeechatString::RE_COLOR_EXT(L"(?:@" + RE_COLOR_ATTRS + L"\\d{5})");
const std::wstring CWeechatString::RE_COLOR_ANY(L"(?:" + RE_COLOR_STD + L"|" + RE_COLOR_EXT + L")");
const std::wregex CWeechatString::RE_COLOR(
#else
const std::string CWeechatString::RE_COLOR_ATTRS("[*!/_|]*");
const std::string CWeechatString::RE_COLOR_STD("(?:" + RE_COLOR_ATTRS + "\\d{2})");
const std::string CWeechatString::RE_COLOR_EXT("(?:@" + RE_COLOR_ATTRS + "\\d{5})");
const std::string CWeechatString::RE_COLOR_ANY("(?:" + RE_COLOR_STD + "|" + RE_COLOR_EXT + ")");
const std::regex CWeechatString::RE_COLOR(
#endif
// \x19: color code, \x1A: set attribute, \x1B: remove attribute, \x1C: reset
    _T("(")
        _T("\x19(?:")
            _T("\\d{2}")
            _T("|F") + RE_COLOR_ANY + _T("")
            _T("|B\\d{2}")
            _T("|B@\\d{5}")
            _T("|E")
            _T("|\\\\*") + RE_COLOR_ANY + _T("(,") + RE_COLOR_ANY + _T(")?")
            _T("|@\\d{5}")
            _T("|b.")
            _T("|\x1C")
        _T(")")
    _T(")")
    _T("|\x1A.")
    _T("|\x1B.")
    _T("|\x1C")
);

CWeechatString::~CWeechatString()
{
}

#ifdef _UNICODE
std::wsregex_token_iterator CWeechatString::begin() const
{
    return std::wsregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
}

std::wsregex_token_iterator CWeechatString::end() const
{
    return std::wsregex_token_iterator();
}
#else
std::sregex_token_iterator CWeechatString::begin() const
{
    return std::sregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
}

std::sregex_token_iterator CWeechatString::end() const
{
    return std::sregex_token_iterator();
}
#endif

CString CWeechatString::decodePlain() const
{
#ifdef _UNICODE
    std::wostringstream plaintext;

    auto begin = std::wsregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
    auto end = std::wsregex_token_iterator();
#else
    std::ostringstream plaintext;

    auto begin = std::sregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
    auto end = std::sregex_token_iterator();
#endif

    for (auto i = begin; i != end; ++i)
    {
#ifdef _UNICODE
        std::wssub_match match = *i;
        std::wstring token = match.str();
#else
        std::ssub_match match = *i;
        std::string token = match.str();
#endif
        switch (token[0])
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
            std::replace(token.begin(), token.end(), '\t', ' ');
            plaintext << token;
            break;
        }
    }

    return CString(plaintext.str().c_str());
}
