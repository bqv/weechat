#include <sstream>

#include "Packet.h"
#include "Types.h"

#define ZLIB_WINAPI
#include "ZLib/zlib.h"

Packet::Packet(std::string bytes)
{
    std::istringstream packetIss(bytes);
    size_t packetRemainingLen = bytes.size();
    length = int_t::read(packetIss, packetRemainingLen).data;
    switch (chr_t::read(packetIss, packetRemainingLen).data)
    {
    case 0x00:
        compression = Compression::NONE;
        break;
    case 0x01:
        compression = Compression::ZLIB;
        break;
    default:
        throw std::invalid_argument("Protocol Error");
    }
    std::string compressedData = ::read_n(packetRemainingLen, packetIss);
    std::string plainData = decompress(compressedData, compression);
    std::istringstream dataIss(plainData);
    size_t remainingLen = plainData.size();
    id = str_t::read(dataIss, remainingLen).data;
    while (dataIss.peek() != EOF)
    {
        std::string type = typ_t::read(dataIss, remainingLen).data;
        obj_t object = ::read(type.c_str(), dataIss, remainingLen);
        objects.push_back(object);
    }
}

std::string Packet::decompress(std::string& str, Compression compression)
{
    switch (compression)
    {
    case Compression::NONE:
    {
        return str;
        break;
    }
    case Compression::ZLIB:
    {
        z_stream zs;                        // z_stream is zlib's control structure
        memset(&zs, 0, sizeof(zs));

        if (inflateInit(&zs) != Z_OK)
            throw(std::runtime_error("inflateInit failed while decompressing."));

        zs.next_in = (Bytef*)str.data();
        zs.avail_in = str.size();

        int ret;
        char outbuffer[32768];
        std::string outstring;

        // get the decompressed bytes blockwise using repeated calls to inflate
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, 0);

            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer,
                    zs.total_out - outstring.size());
            }

        } while (ret == Z_OK);

        inflateEnd(&zs);

        if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
            std::ostringstream oss;
            oss << "Exception during zlib decompression: (" << ret << ") "
                << zs.msg;
            throw(std::runtime_error(oss.str()));
        }
        return outstring;
        break;
    }
    default:
        throw std::invalid_argument("Protocol Error");
        return std::string();
    }
}
