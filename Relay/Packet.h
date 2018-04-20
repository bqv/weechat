#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Types.h"

enum Compression
{
    NONE,
    ZLIB,
};

struct Packet
{
    Packet(std::string bytes);

    int length;
    Compression compression;
    std::optional<std::string> id;
    std::vector<obj_t> objects;
    
    static std::string decompress(std::string& bytes, Compression compression);
};
