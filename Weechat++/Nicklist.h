#pragma once

class CNicklistItem
{
public:
    CNicklistItem();
    ~CNicklistItem();

    CString m_buffer;
    CString m_pointer;
    BOOL m_visible;
    CString m_name;
    CString m_colour;
    CString m_prefix;
    CString m_prefixColour;
};

class CNicklist
{
public:
    CNicklist();
    ~CNicklist();

    CNicklist& operator=(const CNicklist& other);

    CArray<CNicklistItem> m_items;
    CArray<CNicklist> m_sublists;
    CString m_buffer;
    CString m_pointer;
    BOOL m_visible;
    CString m_name;
};
