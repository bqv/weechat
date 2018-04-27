#include "stdafx.h"
#include "Nicklist.h"

CNicklist::CNicklist()
{
}

CNicklist::~CNicklist()
{
}

CNicklist& CNicklist::operator=(const CNicklist& other)
{
    if (this != &other)
    {
        m_items.Append(other.m_items);
        m_sublists.Append(other.m_sublists);
        m_buffer = other.m_buffer;
        m_pointer = other.m_pointer;
        m_visible = other.m_visible;
        m_name = other.m_name;
    }
    return *this;
}

CNicklistItem::CNicklistItem()
{
}

CNicklistItem::~CNicklistItem()
{
}
