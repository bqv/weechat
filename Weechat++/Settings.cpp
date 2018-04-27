#include "stdafx.h"
#include "Settings.h"

Settings::Settings()
{
    font = { -12, 0, 0, 0, FW_NORMAL, FALSE,
        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN,
        _T("Courier New") };
}

Settings::~Settings()
{
}
