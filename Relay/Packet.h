#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Types.h"

struct Packet
{
    Packet(std::string bytes);

    int length;
    bool compression;
    std::optional<std::string> id;
    std::vector<obj_t> objects;
};
