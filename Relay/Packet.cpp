#include <sstream>

#include "Packet.h"
#include "Types.h"

Packet::Packet(std::string bytes)
{
    std::istringstream in(bytes);
    length = int_t::read(in).data;
    compression = chr_t::read(in).data == 0x01;
    id = str_t::read(in).data;
    while (in.peek() != EOF)
    {
        std::string type = typ_t::read(in).data;
        obj_t object = ::read(type.c_str(), in);
        objects.push_back(object);
    }
}
