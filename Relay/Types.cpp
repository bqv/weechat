#include <iterator>
#include <regex>

#include "Types.h"

const typ_t typ_t::read(std::istream & in)
{
    std::string type = ::read_n(3, in);
    return { { type[0], type[1], type[2] } };
}

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
    std::string bytes = ::read_n(4, in);
    int32_t data = static_cast<int>(static_cast<unsigned char>(bytes[0]) << 24 |
        static_cast<unsigned char>(bytes[1]) << 16 |
        static_cast<unsigned char>(bytes[2]) << 8 |
        static_cast<unsigned char>(bytes[3]));
    return { data };
}

const char lon_t::ID[3] = {'l','o','n'};
const lon_t lon_t::read(std::istream & in)
{
    char length = chr_t::read(in).data;
    std::string result = ::read_n(length, in);
    return { std::stol(result) };
}

const char str_t::ID[3] = {'s','t','r'};
const str_t str_t::read(std::istream & in)
{
    int32_t length = int_t::read(in).data;
    if (length == 0xFFFFFFFF)
    {
        return { std::optional<std::string>() };
    }
    std::string result = ::read_n(length, in);
    return { std::optional<std::string>(result) };
}

const char buf_t::ID[3] = {'b','u','f'};
const buf_t buf_t::read(std::istream & in)
{
    int32_t length = int_t::read(in).data;
    if (length == 0xFFFFFFFF)
    {
        return { std::optional<std::string>() };
    }
    std::string result = ::read_n(length, in);
    return { std::optional<std::string>(result) };
}

const char ptr_t::ID[3] = {'p','t','r'};
const ptr_t ptr_t::read(std::istream & in)
{
    char length = chr_t::read(in).data;
    std::string result = ::read_n(length, in);
    return { result };
}

const char tim_t::ID[3] = {'t','i','m'};
const tim_t tim_t::read(std::istream & in)
{
    char length = chr_t::read(in).data;
    std::string result = ::read_n(length, in);
    return { std::stoul(result) };
}

const char htb_t::ID[3] = {'h','t','b'};
const htb_t htb_t::read(std::istream & in)
{
    std::map<std::any, std::any, _compare_any> data;

    std::string keytype = typ_t::read(in).data;
    std::string valuetype = typ_t::read(in).data;

    int count = int_t::read(in).data;

    while (count-- > 0)
    {
        std::any key = ::read(keytype.c_str(), in);
        std::any value = ::read(valuetype.c_str(), in);
        std::pair<std::any, std::any> kvpair(key, value);
        data.insert(kvpair);
    }

    return { data };
}

static const std::regex HDATA_PATH_RGX("(\\w+)");
static const std::regex HDATA_KEYS_RGX("(\\w+):([a-z]{3})");
const char hda_t::ID[3] = {'h','d','a'};
const hda_t hda_t::read(std::istream & in)
{
    std::vector<std::string> hpath;
    std::map<std::string, std::string> keys;
    std::vector<std::pair<std::vector<std::string>, std::vector<std::any>>> values;

    std::string hpath_str = str_t::read(in).data.value();
    auto hpath_begin = std::sregex_iterator(hpath_str.begin(), hpath_str.end(), HDATA_PATH_RGX);
    auto hpath_end = std::sregex_iterator();

    for (std::sregex_iterator i = hpath_begin; i != hpath_end; ++i)
    {
        std::smatch match = *i;
        std::string path = match.str(0);

        hpath.push_back(path);
    }

    std::string keys_str = str_t::read(in).data.value();
    auto keys_begin = std::sregex_iterator(keys_str.begin(), keys_str.end(), HDATA_KEYS_RGX);
    auto keys_end = std::sregex_iterator();

    for (std::sregex_iterator i = keys_begin; i != keys_end; ++i)
    {
        std::smatch match = *i;
        std::string keyname = match.str(1);
        std::string keytype = match.str(2);

        std::pair<std::string, std::string> key(keyname, keytype.substr(0, 3));
        keys.insert(key);
    }

    int count = int_t::read(in).data;

    while (count-- > 0)
    {
        std::vector<std::string> ppath;
        std::vector<std::any> vvalue;

        int n_paths = hpath.size();

        while (n_paths-- > 0)
        {
            ppath.push_back(ptr_t::read(in).data);
        }

        for (const auto& key : keys)
        {
            std::string type = key.second;
            vvalue.push_back(::read(type.c_str(), in));
        }

        std::pair<std::vector<std::string>, std::vector<std::any>> item(ppath, vvalue);
        values.push_back(item);
    }

    return { hpath, keys, values };
}

const char inf_t::ID[3] = {'i','n','f'};
const inf_t inf_t::read(std::istream & in)
{
    std::string name = str_t::read(in).data.value();
    std::string value = str_t::read(in).data.value();

    return { name, value };
}

const char inl_t::ID[3] = {'i','n','l'};
const inl_t inl_t::read(std::istream & in)
{
    std::vector<std::tuple<std::string, std::string, std::any>> items;

    std::string name = str_t::read(in).data.value();
    int count1 = int_t::read(in).data;

    while (count1-- > 0)
    {
        int count2 = int_t::read(in).data;

        while (count2-- > 0)
        {
            std::string name = str_t::read(in).data.value();
            std::string type = typ_t::read(in).data;
            std::any value = ::read(type.c_str(), in);

            items.push_back(std::make_tuple(name, type, value));
        }
    }

    return { name, items };
}

const char arr_t::ID[3] = {'a','r','r'};
const arr_t arr_t::read(std::istream & in)
{
    std::vector<std::any> objects;

    std::string type = typ_t::read(in).data;
    int count = int_t::read(in).data;

    while (count-- > 0)
    {
        std::any value = ::read(type.c_str(), in);

        objects.push_back(value);
    }

    return { { type[0], type[1], type[2] }, objects };
}

const std::any read(const char type[3], std::istream & in)
{
    switch (type[0])
    {
    case 'c':
        if (type[1] == 'h' && type[2] == 'r')
            return chr_t::read(in);
        return std::any();
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
                return std::any();
            }
        return std::any();
    case 'l':
        if (type[1] == 'o' && type[2] == 'n')
            return lon_t::read(in);
        return std::any();
    case 's':
        if (type[1] == 't' && type[2] == 'r')
            return str_t::read(in);
        return std::any();
    case 'b':
        if (type[1] == 'u' && type[2] == 'f')
            return buf_t::read(in);
        return std::any();
    case 'p':
        if (type[1] == 't' && type[2] == 'r')
            return ptr_t::read(in);
        return std::any();
    case 't':
        if (type[1] == 'i' && type[2] == 'm')
            return tim_t::read(in);
        return std::any();
    case 'h':
        if (type[1] == 't' && type[2] == 'b')
            return htb_t::read(in);
        else if (type[1] == 'd' && type[2] == 'a')
            return hda_t::read(in);
        return std::any();
    case 'a':
        if (type[1] == 'r' && type[2] == 'r')
            return arr_t::read(in);
        return std::any();
    default:
        return std::any();
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
    return std::lexicographical_compare(lhs.data.begin(), lhs.data.end(), rhs.data.begin(), rhs.data.end(), _compare_pair_any);
}
inline bool _compare::operator()(const hda_t& lhs, const hda_t& rhs) const
{
    if (lhs.hpath == rhs.hpath)
    {
        if (lhs.keys == rhs.keys)
        {
            if (lhs.values.size() == rhs.values.size() && std::equal(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), _pred_pair_vec_any))
                return false;
            else
                return std::lexicographical_compare(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), rhs.values.end(), _compare_pair_vec_any);
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
        return std::lexicographical_compare(lhs.items.begin(), lhs.items.end(), rhs.items.begin(), rhs.items.end(), _compare_tuple_any);
    }
    else
        return lhs.name > rhs.name;
}
inline bool _compare::operator()(const arr_t& lhs, const arr_t& rhs) const
{
    if (lhs.type == rhs.type)
    {
        if (lhs.values.size() == rhs.values.size() && std::equal(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), _pred_any))
            return false;
        else
            return std::lexicographical_compare(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), rhs.values.end(), _compare_any());
    }
    else
        return lhs.type > rhs.type;
}

bool _compare_any::operator()(const std::any& a, const std::any& b) const
{
    if (a.type() == b.type())
        return false;//std::visit(_compare(), a, b);
    else
        return a.type().hash_code() > b.type().hash_code();
}

bool _compare_vec_any(const std::vector<std::any>& left, const std::vector<std::any>& right)
{
    _compare_any comparator;
    return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(), comparator);
}

bool _compare_pair_vec_any(const std::pair<std::vector<std::string>, std::vector<std::any>>& left, const std::pair<std::vector<std::string>, std::vector<std::any>>& right)
{
    return left.first == right.first && _compare_vec_any(left.second, right.second);
}

bool _compare_pair_any(const std::pair<std::any, std::any>& left, const std::pair<std::any, std::any>& right)
{
    _compare_any comparator;
    return comparator(left.first, right.first) && comparator(left.second, right.second);
}

bool _compare_tuple_any(const std::tuple<std::string, std::string, std::any>& left, const std::tuple<std::string, std::string, std::any>& right)
{
    _compare_any comparator;
    return std::get<0>(left) > std::get<0>(right) && std::get<1>(left) > std::get<1>(right) && comparator(std::get<2>(left), std::get<2>(right));
}

bool _pred_any(const std::any& left, const std::any& right)
{
    _compare_any comparator;
    return comparator(left, right) == comparator(right, left);
}

bool _pred_vec_any(const std::vector<std::any>& left, const std::vector<std::any>& right)
{
    return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(), _pred_any);
}

bool _pred_pair_vec_any(const std::pair<std::vector<std::string>, std::vector<std::any>>& left, const std::pair<std::vector<std::string>, std::vector<std::any>>& right)
{
    return left.first > right.first && _pred_vec_any(left.second, right.second);
}
