#include <iterator>
#include <regex>
#include <stdexcept>

#include "Types.h"

const typ_t typ_t::read(std::istream & in, size_t& maxLen, int depth)
{
    if (maxLen < (size_t)3)
        throw std::invalid_argument("Protocol Error");
    std::string type = ::read_n(3, in);
    maxLen -= 3;
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sType: %s\n"), CString(' ', depth).GetString(), CString(type.substr(0, 3).c_str()));
#endif
    return { { type[0], type[1], type[2] } };
}

const char chr_t::ID[3] = {'c','h','r'};
const chr_t chr_t::read(std::istream & in, size_t& maxLen, int depth)
{
    if (maxLen < (size_t)1)
        throw std::invalid_argument("Protocol Error");
    char data;
    in >> data;
    maxLen -= 1;
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sChar: '%c' : 0x%02X\n"), CString(' ', depth).GetString(), CString((wchar_t)data, 1), data);
#endif
    return { data };
}

const char int_t::ID[3] = {'i','n','t'};
const int_t int_t::read(std::istream & in, size_t& maxLen, int depth)
{
    if (maxLen < (size_t)4)
        throw std::invalid_argument("Protocol Error");
    std::string bytes = ::read_n(4, in);
    maxLen -= 4;
    int32_t data = static_cast<int32_t>(static_cast<unsigned char>(bytes[0]) << 24 |
        static_cast<unsigned char>(bytes[1]) << 16 |
        static_cast<unsigned char>(bytes[2]) << 8 |
        static_cast<unsigned char>(bytes[3]));
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sInt: %d\n"), CString(' ', depth).GetString(), data);
#endif
    return { data };
}

const char lon_t::ID[3] = {'l','o','n'};
const lon_t lon_t::read(std::istream & in, size_t& maxLen, int depth)
{
    char length = chr_t::read(in, maxLen, depth+1).data;
    if (maxLen < (size_t)length)
        throw std::invalid_argument("Protocol Error");
    std::string result = ::read_n(length, in);
    maxLen -= length;
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sLong: %ld\n"), CString(' ', depth).GetString(), std::stol(result));
#endif
    return { std::stol(result) };
}

const char str_t::ID[3] = {'s','t','r'};
const str_t str_t::read(std::istream & in, size_t& maxLen, int depth)
{
    int32_t length = int_t::read(in, maxLen, depth+1).data;
    if (length == 0xFFFFFFFF)
    {
#if defined(WIN32) && defined(_DEBUG)
        TRACE(_T("%sString: NULL\n"), CString(' ', depth).GetString());
#endif
        return { std::optional<std::string>() };
    }
    if (maxLen < (size_t)length)
        throw std::invalid_argument("Protocol Error");
    std::string result = ::read_n(length, in);
    maxLen -= length;
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sString: \"%s\"\n"), CString(' ', depth).GetString(), CString(result.c_str()));
#endif
    return { std::optional<std::string>(result) };
}

const char buf_t::ID[3] = {'b','u','f'};
const buf_t buf_t::read(std::istream & in, size_t& maxLen, int depth)
{
    int32_t length = int_t::read(in, maxLen, depth+1).data;
    if (length == 0xFFFFFFFF)
    {
#if defined(WIN32) && defined(_DEBUG)
        TRACE(_T("%sBuffer: NULL\n"), CString(' ', depth).GetString());
#endif
        return { std::optional<std::string>() };
    }
    if (maxLen < (size_t)length)
        throw std::invalid_argument("Protocol Error");
    std::string result = ::read_n(length, in);
    maxLen -= length;
#if defined(WIN32) && defined(_DEBUG)
    CString out(' ', depth);
    out += _T("Buffer: {");
    for (int n = 0; n < length; ++n)
    {
        CString hex;
        hex.Format(_T("%02X"), result[n]);
        out += hex;
    }
    out += _T("}");
    TRACE(_T("%s \"%s\"\n"), out, CString(result.c_str()));
#endif
    return { std::optional<std::string>(result) };
}

const char ptr_t::ID[3] = {'p','t','r'};
const ptr_t ptr_t::read(std::istream & in, size_t& maxLen, int depth)
{
    char length = chr_t::read(in, maxLen, depth+1).data;
    if (maxLen < (size_t)length)
        throw std::invalid_argument("Protocol Error");
    std::string result = ::read_n(length, in);
    maxLen -= length;
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sPointer: 0x%s\n"), CString(' ', depth).GetString(), CString(result.c_str()));
#endif
    return { "0x" + result };
}

const char tim_t::ID[3] = {'t','i','m'};
const tim_t tim_t::read(std::istream & in, size_t& maxLen, int depth)
{
    char length = chr_t::read(in, maxLen, depth+1).data;
    if (maxLen < (size_t)length)
        throw std::invalid_argument("Protocol Error");
    std::string result = ::read_n(length, in);
    maxLen -= length;
#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sTime: %s\n"), CString(' ', depth).GetString(), CString(result.c_str()));
#endif
    return { std::stoul(result) };
}

const char htb_t::ID[3] = {'h','t','b'};
const htb_t htb_t::read(std::istream & in, size_t& maxLen, int depth)
{
    std::map<obj_t, obj_t, _compare_obj> data;

    std::string keytype(typ_t::read(in, maxLen, depth+1).data, 3);
    std::string valuetype(typ_t::read(in, maxLen, depth+1).data, 3);

    int count = int_t::read(in, maxLen, depth+1).data;

    while (count-- > 0)
    {
        obj_t key = ::read(keytype.c_str(), in, maxLen, depth+1);
        obj_t value = ::read(valuetype.c_str(), in, maxLen, depth+1);
        std::pair<obj_t, obj_t> kvpair(key, value);
        data.insert(kvpair);
    }

#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sHashtable: <%s, %s>\n"), CString(' ', depth).GetString(), CString(keytype.c_str(), 3), CString(valuetype.c_str(), 3));
#endif
    return { data };
}

static const std::regex HDATA_PATH_RGX("(\\w+)");
static const std::regex HDATA_KEYS_RGX("(\\w+):([a-z]{3})");
const char hda_t::ID[3] = {'h','d','a'};
const hda_t hda_t::read(std::istream & in, size_t& maxLen, int depth)
{
    std::vector<std::string> hpath;
    std::vector<std::pair<std::string, std::string>> keys;
    std::vector<std::pair<std::vector<std::string>, std::vector<obj_t>>> values;

    std::string hpath_str = str_t::read(in, maxLen, depth+1).data.value();
    auto hpath_begin = std::sregex_iterator(hpath_str.begin(), hpath_str.end(), HDATA_PATH_RGX);
    auto hpath_end = std::sregex_iterator();

    for (std::sregex_iterator i = hpath_begin; i != hpath_end; ++i)
    {
        std::smatch match = *i;
        std::string path = match.str(0);

        hpath.push_back(path);
    }

    std::string keys_str = str_t::read(in, maxLen, depth+1).data.value();
    auto keys_begin = std::sregex_iterator(keys_str.begin(), keys_str.end(), HDATA_KEYS_RGX);
    auto keys_end = std::sregex_iterator();

    for (std::sregex_iterator i = keys_begin; i != keys_end; ++i)
    {
        std::smatch match = *i;
        std::string keyname = match.str(1);
        std::string keytype = match.str(2);

        std::pair<std::string, std::string> key(keyname, keytype.substr(0, 3));
        keys.push_back(key);
    }

    int count = int_t::read(in, maxLen, depth+1).data;

    while (count-- > 0)
    {
        std::vector<std::string> ppath;
        std::vector<obj_t> vvalue;

        int n_paths = hpath.size();

        while (n_paths-- > 0)
        {
            ppath.push_back(ptr_t::read(in, maxLen, depth+1).data);
        }

        for (const auto& key : keys)
        {
            std::string type = key.second;
            vvalue.push_back(::read(type.c_str(), in, maxLen, depth+1));
        }

        std::pair<std::vector<std::string>, std::vector<obj_t>> item(ppath, vvalue);
        values.push_back(item);
    }

#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sHdata: %s %s\n"), CString(' ', depth).GetString(), CString(hpath_str.c_str()), CString(keys_str.c_str()));
#endif
    return { hpath, keys, values };
}

const char inf_t::ID[3] = {'i','n','f'};
const inf_t inf_t::read(std::istream & in, size_t& maxLen, int depth)
{
    std::string name = str_t::read(in, maxLen, depth+1).data.value();
    std::string value = str_t::read(in, maxLen, depth+1).data.value();

#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sInfo: %s = %s\n"), CString(' ', depth).GetString(), CString(name.c_str()), CString(value.c_str()));
#endif
    return { name, value };
}

const char inl_t::ID[3] = {'i','n','l'};
const inl_t inl_t::read(std::istream & in, size_t& maxLen, int depth)
{
    std::vector<std::tuple<std::string, std::string, obj_t>> items;

    std::string name = str_t::read(in, maxLen, depth+1).data.value();
    int count1 = int_t::read(in, maxLen, depth+1).data;

    while (count1-- > 0)
    {
        int count2 = int_t::read(in, maxLen, depth+1).data;

        while (count2-- > 0)
        {
            std::string name = str_t::read(in, maxLen, depth+1).data.value();
            std::string type = typ_t::read(in, maxLen, depth+1).data;
            obj_t value = ::read(type.c_str(), in, maxLen, depth+1);

            items.push_back(std::make_tuple(name, type, value));
        }
    }

#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sInfolist: %s\n"), CString(' ', depth).GetString(), CString(name.c_str()));
#endif
    return { name, items };
}

const char arr_t::ID[3] = {'a','r','r'};
const arr_t arr_t::read(std::istream & in, size_t& maxLen, int depth)
{
    std::vector<obj_t> objects;

    std::string type = typ_t::read(in, maxLen, depth+1).data;
    int count = int_t::read(in, maxLen, depth+1).data;

    while (count-- > 0)
    {
        obj_t value = ::read(type.c_str(), in, maxLen, depth+1);

        objects.push_back(value);
    }

#if defined(WIN32) && defined(_DEBUG)
    TRACE(_T("%sArray: %s\n"), CString(' ', depth).GetString(), CString(type.substr(0, 3).c_str()));
#endif
    return { { type[0], type[1], type[2] }, objects };
}

const obj_t read(const char type[3], std::istream & in, size_t& maxLen, int depth)
{
    switch (type[0])
    {
    case 'c':
        if (type[1] == 'h' && type[2] == 'r')
            return chr_t::read(in, maxLen, depth);
        return nul_t();
    case 'i':
        if (type[1] == 'n')
            switch (type[2])
            {
            case 't':
                return int_t::read(in, maxLen, depth);
            case 'f':
                return inf_t::read(in, maxLen, depth);
            case 'l':
                return inl_t::read(in, maxLen, depth);
            default:
                return nul_t();
            }
        return nul_t();
    case 'l':
        if (type[1] == 'o' && type[2] == 'n')
            return lon_t::read(in, maxLen, depth);
        return nul_t();
    case 's':
        if (type[1] == 't' && type[2] == 'r')
            return str_t::read(in, maxLen, depth);
        return nul_t();
    case 'b':
        if (type[1] == 'u' && type[2] == 'f')
            return buf_t::read(in, maxLen, depth);
        return nul_t();
    case 'p':
        if (type[1] == 't' && type[2] == 'r')
            return ptr_t::read(in, maxLen, depth);
        return nul_t();
    case 't':
        if (type[1] == 'i' && type[2] == 'm')
            return tim_t::read(in, maxLen, depth);
        return nul_t();
    case 'h':
        if (type[1] == 't' && type[2] == 'b')
            return htb_t::read(in, maxLen, depth);
        else if (type[1] == 'd' && type[2] == 'a')
            return hda_t::read(in, maxLen, depth);
        return nul_t();
    case 'a':
        if (type[1] == 'r' && type[2] == 'r')
            return arr_t::read(in, maxLen, depth);
        return nul_t();
    default:
        return nul_t();
    }
}

const std::string read_n(const size_t length, std::istream & in)
{
    char* buf = new char[length];
    in.read(buf, length);
    std::string result(buf, length);
    delete[] buf;
    return result;
}

bool _compare::operator()(const typ_t & lhs, const typ_t & rhs) const
{ return lhs.data > rhs.data; }
inline bool _compare::operator()(const chr_t& lhs, const chr_t& rhs) const
{ return lhs.data > rhs.data; }
inline bool _compare::operator()(const int_t& lhs, const int_t& rhs) const
{ return lhs.data > rhs.data; }
inline bool _compare::operator()(const lon_t& lhs, const lon_t& rhs) const
{ return lhs.data > rhs.data; }
inline bool _compare::operator()(const str_t& lhs, const str_t& rhs) const
{ return lhs.data.value() > rhs.data.value(); }
inline bool _compare::operator()(const buf_t& lhs, const buf_t& rhs) const
{ return lhs.data.value() > rhs.data.value(); }
inline bool _compare::operator()(const ptr_t& lhs, const ptr_t& rhs) const
{ return lhs.data > rhs.data; }
inline bool _compare::operator()(const tim_t& lhs, const tim_t& rhs) const
{ return lhs.data > rhs.data; }
inline bool _compare::operator()(const htb_t& lhs, const htb_t& rhs) const
{
    return std::lexicographical_compare(lhs.data.begin(), lhs.data.end(), rhs.data.begin(), rhs.data.end(), _compare_pair_obj);
}
inline bool _compare::operator()(const hda_t& lhs, const hda_t& rhs) const
{
    if (lhs.hpath == rhs.hpath)
    {
        if (lhs.keys == rhs.keys)
        {
            if (lhs.values.size() == rhs.values.size() && std::equal(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), _pred_pair_vec_obj))
                return false;
            else
                return std::lexicographical_compare(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), rhs.values.end(), _compare_pair_vec_obj);
        }
        else
            return lhs.keys > rhs.keys;
    }
    else
        return lhs.hpath > rhs.hpath;

}
inline bool _compare::operator()(const inf_t& lhs, const inf_t& rhs) const
{
    if (lhs.name == rhs.name)
    {
        if (lhs.value == rhs.value)
            return false;
        else
            return lhs.value > rhs.value;
    }
    else
        return lhs.name > rhs.name;
}
inline bool _compare::operator()(const inl_t& lhs, const inl_t& rhs) const
{
    if (lhs.name == rhs.name)
    {
        return std::lexicographical_compare(lhs.items.begin(), lhs.items.end(), rhs.items.begin(), rhs.items.end(), _compare_tuple_obj);
    }
    else
        return lhs.name > rhs.name;
}
inline bool _compare::operator()(const arr_t& lhs, const arr_t& rhs) const
{
    if (lhs.type == rhs.type)
    {
        if (lhs.values.size() == rhs.values.size() && std::equal(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), _pred_obj))
            return false;
        else
            return std::lexicographical_compare(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), rhs.values.end(), _compare_obj());
    }
    else
        return lhs.type > rhs.type;
}

bool _compare_obj::operator()(const obj_t& a, const obj_t& b) const
{
    if (a.index() == b.index())
        return std::visit(_compare(), a, b);
    else
        return a.index() > b.index();
}

bool _compare_vec_obj(const std::vector<obj_t>& left, const std::vector<obj_t>& right)
{
    _compare_obj comparator;
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(), comparator);
}

bool _compare_pair_vec_obj(const std::pair<std::vector<std::string>, std::vector<obj_t>>& left, const std::pair<std::vector<std::string>, std::vector<obj_t>>& right)
{
    return left.first == right.first && _compare_vec_obj(left.second, right.second);
}

bool _compare_pair_obj(const std::pair<obj_t, obj_t>& left, const std::pair<obj_t, obj_t>& right)
{
    _compare_obj comparator;
    return comparator(left.first, right.first) && comparator(left.second, right.second);
}

bool _compare_tuple_obj(const std::tuple<std::string, std::string, obj_t>& left, const std::tuple<std::string, std::string, obj_t>& right)
{
    _compare_obj comparator;
    return std::get<0>(left) > std::get<0>(right) && std::get<1>(left) > std::get<1>(right) && comparator(std::get<2>(left), std::get<2>(right));
}

bool _pred_obj(const obj_t& left, const obj_t& right)
{
    _compare_obj comparator;
    return comparator(left, right) == comparator(right, left);
}

bool _pred_vec_obj(const std::vector<obj_t>& left, const std::vector<obj_t>& right)
{
    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), _pred_obj);
}

bool _pred_pair_vec_obj(const std::pair<std::vector<std::string>, std::vector<obj_t>>& left, const std::pair<std::vector<std::string>, std::vector<obj_t>>& right)
{
    return left.first > right.first && _pred_vec_obj(left.second, right.second);
}

const char * ID::operator()(const chr_t & lhs) const
{
    return chr_t::ID;
}

const char * ID::operator()(const int_t & lhs) const
{
    return int_t::ID;
}

const char * ID::operator()(const lon_t & lhs) const
{
    return lon_t::ID;
}

const char * ID::operator()(const str_t & lhs) const
{
    return str_t::ID;
}

const char * ID::operator()(const buf_t & lhs) const
{
    return buf_t::ID;
}

const char * ID::operator()(const ptr_t & lhs) const
{
    return ptr_t::ID;
}

const char * ID::operator()(const tim_t & lhs) const
{
    return tim_t::ID;
}

const char * ID::operator()(const htb_t & lhs) const
{
    return htb_t::ID;
}

const char * ID::operator()(const hda_t & lhs) const
{
    return hda_t::ID;
}

const char * ID::operator()(const inf_t & lhs) const
{
    return inf_t::ID;
}

const char * ID::operator()(const inl_t & lhs) const
{
    return inl_t::ID;
}

const char * ID::operator()(const arr_t & lhs) const
{
    return arr_t::ID;
}

const char * ID::operator()(const nul_t & lhs) const
{
    return nullptr;
}
