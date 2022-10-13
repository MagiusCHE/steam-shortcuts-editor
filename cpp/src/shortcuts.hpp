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
