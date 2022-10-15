/*
 * Copyright (c) 2022, Magius(CHE)
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 * Read the LICENSE file for more details.
 *
 * @author: Magius(CHE) - magiusche@magius.it
 */

#pragma once
#include <fmt/core.h>

#include <cstdarg>
#include <iostream>
#include <string>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <boost/filesystem.hpp>
#include <chrono>
#include <ctime>
#include <filesystem>

using namespace std;

#define debug$(format, ...) \
    logging::debug(__FILE__, __LINE__, FMT_STRING(format), __VA_ARGS__)

#define log$(sender, format, ...) \
    logging::log(sender, FMT_STRING(format), __VA_ARGS__)

#define error$(sender, format, ...) \
    logging::error(sender, FMT_STRING(format), __VA_ARGS__)

const bool _isatty = isatty(fileno(stderr)) && isatty(fileno(stdin)) && isatty(fileno(stdout));

namespace logging {
namespace {

static int terminal_colors[] = {20, 21, 26, 27, 32, 33, 38, 39, 40, 41, 42, 43, 44, 45, 56, 57, 62, 63, 68, 69, 74, 75, 76, 77, 78, 79, 80, 81, 92, 93, 98, 99, 112, 113, 128, 129, 134, 135, 148, 149, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 178, 179, 184, 185, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 214, 215, 220, 221};

int findLastIndex(const char *str, char x) {
    for (int i = strlen(str) - 1; i >= 0; i--)
        if (str[i] == x) {
            return i;
        }
    return -1;
}

int log_color_by_hash(const string &text) {
    int32_t hash = 0;

    for (size_t i = 0; i < text.length(); i++) {
        hash = ((hash << 5) - hash) + text.at(i);
    }

    // fmt::print("Colors: {}\n", hash);

    static const size_t size = sizeof(terminal_colors) / sizeof(int32_t);

    return terminal_colors[(size_t)(abs(hash) % size)];
}

static auto initial_time = std::chrono::system_clock::now();
static auto last_log_time = std::chrono::system_clock::now();
static auto src_rootpath_size = findLastIndex(__FILE__, '/');

void vlog(const string &sender, bool space, int error, fmt::string_view format,
          fmt::format_args args) {
    auto std = error != 0 ? stderr : stdout;

    auto color = log_color_by_hash(sender);

    auto colorCode = fmt::format("\u001B[3{}{}", (color < 8 ? "" : "8;5;"), color);

    last_log_time = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = last_log_time - initial_time;

    if (_isatty)
        fmt::print(std, "{};1m{}{}\u001B[0m", colorCode, sender, space ? " " : "");
    else
        fmt::print(std, "{}{}", sender, space ? " " : "");

    fmt::vprint(std, format, args);

    auto passed = elapsed_seconds.count();
    fmt::print(std, " {}{}{}{}s{}\n",
               _isatty ? colorCode : "",
               _isatty ? "m+" : "",
               passed < 1000 ? floor(passed * 1000.0) : (floor(passed * 100.0) / 100.0),
               passed < 1000 ? "m" : "",
               _isatty ? "\u001B[0m" : "");
}
}  // namespace
template <typename S, typename... Args>
inline void debug(const string &file, int line, const S &format, Args &&...args) {
    assert(file.length() >= src_rootpath_size);

    auto file_str = std::filesystem::canonical(__FILE__).string().substr(src_rootpath_size);

    vlog(file_str, false, 0, fmt::format(":{} {}", line, format), fmt::make_format_args(args...));
}
template <typename S, typename... Args>
inline void log(const string &sender, const S &format, Args &&...args) {
    vlog(sender, true, 0, format, fmt::make_format_args(args...));
}

template <typename S, typename... Args>
inline void error(const string &sender, const S &format, Args &&...args) {
    vlog(sender, true, -1, format, fmt::make_format_args(args...));
}

inline void update_debug_src_root_path(const char *file) {
    // auto str = std::filesystem::canonical(file).string();
    src_rootpath_size = findLastIndex(file /*str.c_str()*/, '/');
    // log$("logger", "_isatty {}, {}", _isatty, fileno(stderr));
}
}  // namespace logging

class Loggable {
    string log_name;

   public:
    Loggable(string log_name) {
        Loggable::log_name = log_name;
        // log("{}", "Constructed");
    }

   protected:
    template <typename S, typename... Args>
    void log(const S &format, Args &&...args) const {
        logging::vlog(log_name, true, 0, format, fmt::make_format_args(args...));
    }

    template <typename S, typename... Args>
    void error(const S &format, Args &&...args) const {
        logging::vlog(log_name, true, -1, format, fmt::make_format_args(args...));
    }
};