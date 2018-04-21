#pragma once

class CWeechatString
{
public:
    template<typename... Args>
    CWeechatString(Args... args)
    {
        m_raw = CString(args...);
    }
    ~CWeechatString();

    CString m_raw;

    template<typename T = CString>
    CWeechatString& operator=(const T src)
    {
        m_raw = src;
        return *this;
    }

    template<typename T = CString>
    CWeechatString& operator+=(const T src)
    {
        m_raw += src;
        return *this;
    }

    template<typename T = CString>
    CWeechatString operator+(const T rhs) const
    {
        return CWeechatString(m_raw + rhs);
    }

    template<typename T = CString>
    T GetRaw() const
    {
        return m_raw;
    }

    template<typename T = CStringA>
    T GetRtf() const
    {
        CStringA rtf = decode();
        return rtf;
    }
    
private:
    CStringA decode() const;
};
