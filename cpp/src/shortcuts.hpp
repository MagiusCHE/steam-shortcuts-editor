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

    using Value = variant<uint32_t, string, Map>;
    using VdfMap = map<string, Value>;

    struct Map
    {
        VdfMap v;
    };

    class Shortcut : private Loggable
    {
    public:
        Shortcut();
        ~Shortcut()
        {
        }
    };

    //
    class Shortcuts : private Loggable
    {

    private:
        fs::path source_file;
        size_t buffer_size;
        size_t buffer_index;
        optional<Map> consume_map();
        bool eof();
        optional<tuple<string, Value>> consume_map_item();
        Shortcuts();

    public:
        Shortcuts(const string source);
        Shortcuts(const fs::path source);
        // const int parse() const;
        int parse();
        ~Shortcuts()
        {
        }
    };

}
