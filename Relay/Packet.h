#pragma once

#include <string>
#include <vector>
#include <any>
#include <optional>

struct Packet
{
    Packet(std::string bytes);

    int length;
    bool compression;
    std::optional<std::string> id;
    std::vector<std::any> objects;
};
