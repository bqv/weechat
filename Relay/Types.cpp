#include <iterator>

#include "Types.h"

const char chr_t::ID[3] = {'c','h','r'};
const chr_t chr_t::read(std::istream & in)
{
    char data;
    in >> data;
    return { data };
}

const char int_t::ID[3] = {'i','n','t'};
const int_t int_t::read(std::istream & in)
{
    int32_t data;
    in >> data;
    return { data };
}

const char lon_t::ID[3] = {'l','o','n'};
const lon_t lon_t::read(std::istream & in)
{
    char length;
    in >> length;
    std::string result;
    result.reserve(length);
    std::copy_n(std::istreambuf_iterator<char>(in), length, std::back_inserter(result));
    return { std::stol(result) };
}

const char str_t::ID[3] = {'s','t','r'};
const str_t str_t::read(std::istream & in)
{
    int32_t length;
    in >> length;
    std::string result;
    result.reserve(length);
    std::copy_n(std::istreambuf_iterator<char>(in), length, std::back_inserter(result));
    return { result };
}

const char buf_t::ID[3] = {'b','u','f'};
const buf_t buf_t::read(std::istream & in)
{
    int32_t length;
    in >> length;
    std::string result;
    result.reserve(length);
    std::copy_n(std::istreambuf_iterator<char>(in), length, std::back_inserter(result));
    return { result };
}

const char ptr_t::ID[3] = {'p','t','r'};
const ptr_t ptr_t::read(std::istream & in)
{
    char length;
    in >> length;
    std::string result;
    result.reserve(length);
    std::copy_n(std::istreambuf_iterator<char>(in), length, std::back_inserter(result));
    return { result };
}

const char tim_t::ID[3] = {'t','i','m'};
const tim_t tim_t::read(std::istream & in)
{
    char length;
    in >> length;
    std::string result;
    result.reserve(length);
    std::copy_n(std::istreambuf_iterator<char>(in), length, std::back_inserter(result));
    return { std::stoul(result) };
}

const char htb_t::ID[3] = {'h','t','b'};
const htb_t htb_t::read(std::istream & in)
{
    std::map<obj_t, obj_t> data;

    std::string keytype, valuetype;
    keytype.reserve(3);
    valuetype.reserve(3);
    std::copy_n(std::istreambuf_iterator<char>(in), 3, std::back_inserter(keytype));
    std::copy_n(std::istreambuf_iterator<char>(in), 3, std::back_inserter(valuetype));

    int count = int_t::read(in).data;

    while (count-- > 0)
    {
        obj_t key = ::read(keytype.c_str(), in);
        obj_t value = ::read(valuetype.c_str(), in);
        std::pair<obj_t, obj_t> kvpair(key, value);
        data.insert(kvpair);
    }

    return { data };
}

const char hda_t::ID[3] = {'h','d','a'};
const hda_t hda_t::read(std::istream & in)
{
}

const char inf_t::ID[3] = {'i','n','f'};
const inf_t inf_t::read(std::istream & in)
{
}

const char inl_t::ID[3] = {'i','n','l'};
const inl_t inl_t::read(std::istream & in)
{
}

const char arr_t::ID[3] = {'a','r','r'};
const arr_t arr_t::read(std::istream & in)
{
}

const obj_t read(const char type[3], std::istream & in)
{
    switch (type[0])
    {
    case 'c':
        if (type[1] == 'h' && type[2] == 'r')
            return chr_t::read(in);
        return nul_t();
    case 'i':
        if (type[1] == 'n')
            switch (type[2])
            {
            case 't':
                return int_t::read(in);
            case 'f':
                return inf_t::read(in);
            case 'l':
                return inl_t::read(in);
            default:
                return nul_t();
            }
        return nul_t();
    case 'l':
        if (type[1] == 'o' && type[2] == 'n')
            return lon_t::read(in);
        return nul_t();
    case 's':
        if (type[1] == 't' && type[2] == 'r')
            return str_t::read(in);
        return nul_t();
    case 'b':
        if (type[1] == 'u' && type[2] == 'f')
            return buf_t::read(in);
        return nul_t();
    case 'p':
        if (type[1] == 't' && type[2] == 'r')
            return ptr_t::read(in);
        return nul_t();
    case 't':
        if (type[1] == 'i' && type[2] == 'm')
            return tim_t::read(in);
        return nul_t();
    case 'h':
        if (type[1] == 't' && type[2] == 'b')
            return htb_t::read(in);
        else if (type[1] == 'd' && type[2] == 'a')
            return hda_t::read(in);
        return nul_t();
    case 'a':
        if (type[1] == 'r' && type[2] == 'r')
            return arr_t::read(in);
        return nul_t();
    default:
        return nul_t();
    }
}
