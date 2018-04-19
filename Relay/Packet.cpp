#include <sstream>

#include "Packet.h"
#include "Types.h"

Packet::Packet(std::string bytes)
{
    std::istringstream in(bytes);
    size_t maxLen = bytes.size();
    length = int_t::read(in, maxLen).data;
    compression = chr_t::read(in, maxLen).data == 0x01;
    id = str_t::read(in, maxLen).data;
    while (in.peek() != EOF)
    {
        std::string type = typ_t::read(in, maxLen).data;
        obj_t object = ::read(type.c_str(), in, maxLen);
        objects.push_back(object);
    }
}
