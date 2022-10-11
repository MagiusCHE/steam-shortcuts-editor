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

#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <variant>
#include <tuple>
#include <optional>

#include "./logging.hpp"

namespace fs = filesystem;
namespace shortcuts
{
    // class Value;
    struct Map;

    using Value = variant<uint32_t, string, shortcuts::Map>;
    using VdfMap = map<string, Value>;

    // FIXME: remove this structore and point directly to VdfMap
    struct Map
    {
        VdfMap v;
    };

    class Shortcut : private Loggable
    {
    private:
    public:
        // Touble (map) request default empty constructor???
        Shortcut();
        Shortcut(uint32_t index, const shortcuts::Map &map);
        ~Shortcut()
        {
        }
        //unordered_map<string, variant<string, uint32_t>> props;
        map<string, variant<string, uint32_t>> props;
    };

    //
    class Shortcuts : private Loggable
    {

    private:
        fs::path source_file;
        size_t buffer_size;

        // FIXME: remove this buffer_index in favor of ifstream.tellg()
        size_t buffer_index;
        optional<Map> consume_map(ifstream &file);
        bool eof();
        optional<tuple<string, Value>> consume_map_item(ifstream &file);
        Shortcuts();
        optional<uint8_t> consume_byte(ifstream &file);
        optional<string> consume_string(ifstream &file);
        optional<uint32_t> consume_uint32(ifstream &file);
        optional<uint8_t> peek_byte(ifstream &file);
        map<uint32_t, Shortcut> shortcuts;

    public:
        Shortcuts(const string source);
        Shortcuts(const fs::path source);
        // const int parse() const;
        int parse();
        void foreach (function<bool(const Shortcut &sc)> f);
        ~Shortcuts()
        {
        }
    };

}
