#pragma once

#include <string_view>
#include <span>
#include <cstdio>
#include <algorithm>


template <typename T>
using pair = std::pair<T, T>;

using std::string_view;

using byte = char8_t;

using bytes = std::u8string;

using bytes_view = std::u8string_view;

using char_map = std::array<bytes, 256>;


using c_string = const char*;


struct arg_t {
    std::string_view name = "";
    std::string_view flag = "";
    std::string_view description = "";
    std::string_view value = "";
    bool optional = false;
};

/// @return [ input file, output file ]
template <size_t _size>
inline std::array<c_string, _size> extract_args(
    std::span<c_string> argv, 
    std::span<arg_t, _size> req_args
) {
    std::array<c_string, _size> result;
    for (auto& req_arg : req_args) {
        // se le suma uno dado que los flags se pasan por ejemplo: -i file
        auto flag_ptr = std::find(argv.begin(), argv.end(), req_arg.flag) + 1;
        if (argv.begin() < flag_ptr && flag_ptr < argv.end())
            req_arg.value = *flag_ptr;
    }

    for (size_t i = 0; i < _size; ++i) {
        result[i] = req_args[i].value.data();
    }

    return result;
}

void show_help(std::span<arg_t> args);

void program(std::span<c_string> argv);

bytes compress(bytes const& file_content);
bytes uncompress(bytes const& file_content);