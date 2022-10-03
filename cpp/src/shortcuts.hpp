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
        Shortcut(string index, const shortcuts::Map &map);
        ~Shortcut()
        {
        }
        map<string, variant<string, uint32_t>> props;
        string index;
        /*string devkit_game_id;
        uint32_t open_vr;
        string launch_options;
        string exe;
        string icon;
        uint32_t devkit;
        string flatpak_app_id;
        string start_dir;
        uint32_t allow_desktop_config;
        string appname;
        uint32_t appid;
        string shortcut_path;
        uint32_t is_hidden;
        uint32_t allow_overlay;
        uint32_t devkit_override_app_id;
        vector<string> tags;
        uint32_t last_play_time;*/
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
        map<string, Shortcut> shortcuts;

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
