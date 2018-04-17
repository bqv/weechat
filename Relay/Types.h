#pragma once

#include <string>
#include <variant>
#include <map>
#include <vector>

struct chr_t;
struct int_t;
struct lon_t;
struct str_t;
struct buf_t;
struct ptr_t;
struct tim_t;
struct htb_t;
struct hda_t;
struct inf_t;
struct inl_t;
struct arr_t;

struct nul_t {};

typedef std::variant<chr_t, int_t, lon_t, str_t, buf_t, ptr_t, tim_t, htb_t, hda_t, inf_t, inl_t, arr_t, nul_t> obj_t;

struct compare {
    bool operator() (const obj_t& a, const obj_t& b) const {
        if (a.index() == b.index())
            return false;
        else
            return a.index() > b.index();
    }
};

const obj_t read(const char type[3], std::istream& in);

struct chr_t
{
    static const char ID[3];
    const char data;

    static const chr_t read(std::istream& in);
};

struct int_t
{
    static const char ID[3];
    const int data;

    static const int_t read(std::istream& in);
};

struct lon_t
{
    static const char ID[3];
    const long data;

    static const lon_t read(std::istream& in);
};

struct str_t
{
    static const char ID[3];
    const std::string data;

    static const str_t read(std::istream& in);
};

struct buf_t
{
    static const char ID[3];
    const std::string data;

    static const buf_t read(std::istream& in);
};

struct ptr_t
{
    static const char ID[3];
    const std::string data;

    static const ptr_t read(std::istream& in);
};

struct tim_t
{
    static const char ID[3];
    const unsigned long data;

    static const tim_t read(std::istream& in);
};

struct htb_t
{
    static const char ID[3];
    const std::map<obj_t, obj_t, compare> data;

    static const htb_t read(std::istream& in);
};

struct hda_t
{
    static const char ID[3];
    const std::string hpath;
    const std::map<std::string, char[3]> keys;
    const std::vector<std::vector<obj_t>> values;

    static const hda_t read(std::istream& in);
};

struct inf_t
{
    static const char ID[3];
    const std::string name;
    const std::string value;

    static const inf_t read(std::istream& in);
};

struct inl_t
{
    static const char ID[3];
    const std::vector<std::tuple<std::string, char[3], obj_t>> data;

    static const inl_t read(std::istream& in);
};

struct arr_t
{
    static const char ID[3];
    const char type[3];
    const std::vector<obj_t> values;

    static const arr_t read(std::istream& in);
};

class obj_compare
{
    bool operator()(const chr_t& lhs, const chr_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const int_t& lhs, const int_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const lon_t& lhs, const lon_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const str_t& lhs, const str_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const ptr_t& lhs, const ptr_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const tim_t& lhs, const tim_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const htb_t& lhs, const htb_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const hda_t& lhs, const hda_t& rhs) const
    {
        if (lhs.hpath == rhs.hpath)
        {
            if (lhs.keys == rhs.keys)
            {
                if (lhs.values == rhs.values)
                    return false;
                else
                    return lhs.values > rhs.values;
            }
            else
                return lhs.keys > rhs.keys;
        }
        else
            return lhs.hpath > rhs.hpath;

    }
    bool operator()(const inf_t& lhs, const inf_t& rhs) const
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
    bool operator()(const inl_t& lhs, const inl_t& rhs) const
    { return lhs.data > rhs.data; }
    bool operator()(const arr_t& lhs, const arr_t& rhs) const
    {
        if (lhs.type == rhs.type)
        {
            if (lhs.values == rhs.values)
                return false;
            else
                return lhs.values > rhs.values;
        }
        else
            return lhs.type > rhs.type;
    }
    bool operator()(const nul_t& lhs, const nul_t& rhs) const
    { return false; }
};

/*
struct compare {
    bool operator() (const obj_t& a, const obj_t& b) const {
        if (a.index() == b.index())
            std::visit(obj_compare(), a, b);
        else
            return a.index() > b.index();
    }
};
*/
