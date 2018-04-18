#pragma once

#include <string>
#include <any>
#include <map>
#include <vector>
#include <algorithm>
#include <optional>

struct typ_t;
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

struct _compare
{
    bool operator()(const typ_t& lhs, const typ_t& rhs) const;
    bool operator()(const chr_t& lhs, const chr_t& rhs) const;
    bool operator()(const int_t& lhs, const int_t& rhs) const;
    bool operator()(const lon_t& lhs, const lon_t& rhs) const;
    bool operator()(const str_t& lhs, const str_t& rhs) const;
    bool operator()(const buf_t& lhs, const buf_t& rhs) const;
    bool operator()(const ptr_t& lhs, const ptr_t& rhs) const;
    bool operator()(const tim_t& lhs, const tim_t& rhs) const;
    bool operator()(const htb_t& lhs, const htb_t& rhs) const;
    bool operator()(const hda_t& lhs, const hda_t& rhs) const;
    bool operator()(const inf_t& lhs, const inf_t& rhs) const;
    bool operator()(const inl_t& lhs, const inl_t& rhs) const;
    bool operator()(const arr_t& lhs, const arr_t& rhs) const;
};

struct _compare_any
{
    bool operator() (const std::any& a, const std::any& b) const;
};

bool _compare_vec_any(const std::vector<std::any>& left, const std::vector<std::any>& right);
bool _compare_pair_vec_any(const std::pair<std::vector<std::string>, std::vector<std::any>>& left, const std::pair<std::vector<std::string>, std::vector<std::any>>& right);
bool _compare_pair_any(const std::pair<std::any, std::any>& left, const std::pair<std::any, std::any>& right);
bool _compare_tuple_any(const std::tuple<std::string, std::string, std::any>& left, const std::tuple<std::string, std::string, std::any>& right);
bool _pred_any(const std::any& left, const std::any& right);
bool _pred_vec_any(const std::vector<std::any>& left, const std::vector<std::any>& right);
bool _pred_pair_vec_any(const std::pair<std::vector<std::string>, std::vector<std::any>>& left, const std::pair<std::vector<std::string>, std::vector<std::any>>& right);

const std::any read(const char type[3], std::istream& in);
const std::string read_n(const size_t length, std::istream& in);

struct typ_t
{
    const char data[3];

    static const typ_t read(std::istream& in);
};

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
    const std::optional<std::string> data;

    static const str_t read(std::istream& in);
};

struct buf_t
{
    static const char ID[3];
    const std::optional<std::string> data;

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
    const std::map<std::any, std::any, _compare_any> data;

    static const htb_t read(std::istream& in);
};

struct hda_t
{
    static const char ID[3];
    const std::vector<std::string> hpath;
    const std::map<std::string, std::string> keys;
    const std::vector<std::pair<std::vector<std::string>, std::vector<std::any>>> values;

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
    const std::string name;
    const std::vector<std::tuple<std::string, std::string, std::any>> items;

    static const inl_t read(std::istream& in);
};

struct arr_t
{
    static const char ID[3];
    const char type[3];
    const std::vector<std::any> values;

    static const arr_t read(std::istream& in);
};
