#pragma once

#include <string>
#include <vector>

class Util
{
public:
    Util() = delete;
    virtual ~Util() = delete;

    static std::string join(const std::vector<std::string>& elements, const char* const separator);
};
