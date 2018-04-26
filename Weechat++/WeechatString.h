#pragma once

#include <regex>
#include <string>

class CWeechatString
{
protected:
#ifdef _UNICODE
    static const std::wstring CWeechatString::RE_COLOR_ATTRS;
    static const std::wstring CWeechatString::RE_COLOR_STD;
    static const std::wstring CWeechatString::RE_COLOR_EXT;
    static const std::wstring CWeechatString::RE_COLOR_ANY;
    static const std::wregex CWeechatString::RE_COLOR;
#else
    static const std::string CWeechatString::RE_COLOR_ATTRS;
    static const std::string CWeechatString::RE_COLOR_STD;
    static const std::string CWeechatString::RE_COLOR_EXT;
    static const std::string CWeechatString::RE_COLOR_ANY;
    static const std::regex CWeechatString::RE_COLOR;
#endif
    
public:
    template<typename... Args>
    CWeechatString(Args... args)
    {
        m_raw = CString(args...);
#ifdef _UNICODE
        m_wctext = CT2W(m_raw);
#else
        m_wctext = CT2A(m_raw);
#endif
    }
    ~CWeechatString();

    CString m_raw;
#ifdef _UNICODE
    std::wstring m_wctext;
#else
    std::string m_wctext;
#endif

    template<typename T = CString>
    CWeechatString& operator=(const T src)
    {
        m_raw = src;
#ifdef _UNICODE
        m_wctext = CT2W(m_raw);
#else
        m_wctext = CT2A(m_raw);
#endif
        return *this;
    }

    template<typename T = CString>
    CWeechatString& operator+=(const T src)
    {
        m_raw += src;
#ifdef _UNICODE
        m_wctext = CT2W(m_raw);
#else
        m_wctext = CT2A(m_raw);
#endif
        return *this;
    }

    template<typename T = CString>
    CWeechatString operator+(const T rhs) const
    {
        return CWeechatString(m_raw + rhs);
    }

#ifdef _UNICODE
    std::wsregex_token_iterator begin() const;
    std::wsregex_token_iterator end() const;
#else
    std::sregex_token_iterator begin() const;
    std::sregex_token_iterator end() const;
#endif

    template<typename T = CString>
    const T GetRaw() const
    {
        return m_raw;
    }

    template<typename T = CString>
    const T GetPlain() const
    {
        CString plain = decodePlain();
        return plain;
    }
    
private:
    CString decodePlain() const;
};
