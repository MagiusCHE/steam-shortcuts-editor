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
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "./logging.hpp"

namespace fs = filesystem;
namespace shortcuts {

enum PropsValidReturns {
    InvalidKey,
    InvalidValue,
    InvalidArrayValue,
    Ok,
};

enum PropsTypes {
    UInt32,
    String,
    Strings,
};

// class Value;
struct Map;

using Value = variant<uint32_t, string, shortcuts::Map>;
using VdfMap = map<string, Value>;

// FIXME: remove this structore and point directly to VdfMap
struct Map {
    VdfMap v;
};

using PropsType = map<string, tuple<string, PropsTypes, uint8_t>>;

const PropsType valid_props = {
    {"index", {"Index", PropsTypes::UInt32, 0}},
    {"app_id", {"AppId", PropsTypes::UInt32, 1}},
    {"app_name", {"AppName", PropsTypes::String, 2}},
    {"exe", {"Exe", PropsTypes::String, 3}},
    {"start_dir", {"StartDir", PropsTypes::String, 4}},
    {"icon", {"icon", PropsTypes::String, 5}},
    {"shortcut_path", {"ShortcutPath", PropsTypes::String, 6}},
    {"launch_options", {"LaunchOptions", PropsTypes::String, 7}},
    {"is_hidden", {"IsHidden", PropsTypes::UInt32, 8}},
    {"allow_desktop_config", {"AllowDesktopConfig", PropsTypes::UInt32, 9}},
    {"allow_overlay", {"AllowOverlay", PropsTypes::UInt32, 10}},
    {"open_vr", {"OpenVR", PropsTypes::UInt32, 11}},
    {"devkit", {"Devkit", PropsTypes::UInt32, 12}},
    {"devkit_game_id", {"DevkitGameID", PropsTypes::String, 13}},
    {"devkit_override_app_id", {"DevkitOverrideAppID", PropsTypes::UInt32, 14}},
    {"last_play_time", {"LastPlayTime", PropsTypes::UInt32, 15}},
    {"flatpak_app_id", {"FlatpakAppID", PropsTypes::String, 16}},
    {"tags", {"Tags", PropsTypes::Strings, 17}}};

static const uint8_t MAX_VALID_PROPS = 18;

using PropValue = variant<string, uint32_t>;
using Props = map<string, PropValue>;

class Shortcut : private Loggable {
  private:
  public:
    // Touble (map) request default empty constructor???
    Shortcut();
    Shortcut(uint32_t index, const shortcuts::Map &map);
    ~Shortcut() {
    }
    // unordered_map<string, variant<string, uint32_t>> props;
    Props props;
    void store_into(ofstream &out) const;
};

//
class Shortcuts : private Loggable {
  private:
    size_t buffer_size;

    // FIXME: remove this buffer_index in favor of ifstream.tellg()
    size_t buffer_index;
    optional<Map> consume_map(ifstream &file);
    bool eof();
    optional<tuple<string, Value>> consume_map_item(ifstream &file);

    optional<uint8_t> consume_byte(ifstream &file);
    optional<string> consume_string(ifstream &file);
    optional<uint32_t> consume_uint32(ifstream &file);
    optional<uint8_t> peek_byte(ifstream &file);
    map<uint32_t, Shortcut> shortcuts;

  public:
    Shortcuts();
    static bool prop_is_uint32(const string &name);
    static bool prop_is_string(const string &name);
    static bool prop_is_stringarr(const string &name);
    static string resja(const string &json_stringified_array, bool emptystring_if_emptyarray = false);
    static PropsValidReturns is_prop_valid(const string &key, const string &value, bool ignore_value_check = false);
    static void for_each_valid_props(function<void(const PropsType &prop)> f);

    template <typename T>
    static optional<tuple<string, T>> prop_optional_find(const map<string, T> &props, function<bool(const T &elem)> test);
    template <typename T>
    static optional<T> prop_optional_find(const map<string, T> &props, const string &key);

    // const int parse() const;
    int parse(string source) {
        return parse(fs::path(source));
    }

    int parse(const fs::path &source);
    int update_from_json_file(const fs::path &source);
    int update_from_json(const string &source);

    void foreach (function<bool(const Shortcut &sc)> f);
    // Shortcut * at(uint32_t index);
    bool get_or_create(uint32_t index, function<void(bool isnew, Shortcut &sc)> f);
    void store_into(const string &destfile);
    ~Shortcuts() {}
};

} // namespace shortcuts
