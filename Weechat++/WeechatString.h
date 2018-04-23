#pragma once

#include <regex>
#include <string>

class CWeechatString
{
protected:
    static const std::string CWeechatString::RE_COLOR_ATTRS;
    static const std::string CWeechatString::RE_COLOR_STD;
    static const std::string CWeechatString::RE_COLOR_EXT;
    static const std::string CWeechatString::RE_COLOR_ANY;
    static const std::regex CWeechatString::RE_COLOR;
    
public:
    template<typename... Args>
    CWeechatString(Args... args)
    {
        m_raw = CString(args...);
        m_wctext = CT2A(m_raw);
    }
    ~CWeechatString();

    CString m_raw;
    std::string m_wctext;

    template<typename T = CString>
    CWeechatString& operator=(const T src)
    {
        m_raw = src;
        m_wctext = CT2A(m_raw);
        return *this;
    }

    template<typename T = CString>
    CWeechatString& operator+=(const T src)
    {
        m_raw += src;
        m_wctext = CT2A(m_raw);
        return *this;
    }

    template<typename T = CString>
    CWeechatString operator+(const T rhs) const
    {
        return CWeechatString(m_raw + rhs);
    }

    std::sregex_token_iterator begin() const;
    std::sregex_token_iterator end() const;

    template<typename T = CString>
    T GetRaw() const
    {
        return m_raw;
    }

    template<typename T = CStringA>
    T GetRtf() const
    {
        CStringA rtf = decodeRtf();
        return rtf;
    }

    template<typename T = CString>
    T GetPlain() const
    {
        CString plain = decodePlain();
        return plain;
    }
    
private:
    CStringA decodeRtf() const;
    CString decodePlain() const;
};
