/*
 * Copyright (c) 2022, Magius(CHE)
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 * Read the LICENSE file for more details.
 *
 * @author: Magius(CHE) - magiusche@magius.it
 */

#include "./shortcuts.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <codecvt>
#include <fstream>
#include <locale>
#include <regex>
#include <sstream>

using namespace std;
using namespace shortcuts;

namespace fs = filesystem;

namespace json = boost::json;

using PropsType = map<string, tuple<string, PropsTypes, uint8_t>>;

const PropsType valid_props = {
    {"app_id", {"AppId", PropsTypes::UInt32, 0}},
    {"app_name", {"AppName", PropsTypes::String, 1}},
    {"exe", {"Exe", PropsTypes::String, 2}},
    {"start_dir", {"StartDir", PropsTypes::String, 3}},
    {"icon", {"icon", PropsTypes::String, 4}},
    {"shortcut_path", {"ShortcutPath", PropsTypes::String, 5}},
    {"launch_options", {"LaunchOptions", PropsTypes::String, 6}},
    {"is_hidden", {"IsHidden", PropsTypes::UInt32, 7}},
    {"allow_desktop_config", {"AllowDesktopConfig", PropsTypes::UInt32, 8}},
    {"allow_overlay", {"AllowOverlay", PropsTypes::UInt32, 9}},
    {"open_vr", {"OpenVR", PropsTypes::UInt32, 10}},
    {"devkit", {"Devkit", PropsTypes::UInt32, 11}},
    {"devkit_game_id", {"DevkitGameID", PropsTypes::String, 12}},
    {"devkit_override_app_id", {"DevkitOverrideAppID", PropsTypes::UInt32, 13}},
    {"last_play_time", {"LastPlayTime", PropsTypes::UInt32, 14}},
    {"flatpak_app_id", {"FlatpakAppID", PropsTypes::String, 15}},
    {"tags", {"Tags", PropsTypes::Strings, 16}}};

static const uint8_t MAX_VALID_PROPS = 17;

enum VdfMapItemType : uint8_t {
    Map = 0x00,
    String = 0x01,
    UInt32 = 0x02,
    MapEnd = 0x08,
};

template <typename T>
optional<T> prop_optional_find(const map<string, T> &props, const string &key) {
    // log$("prop_optional_find", "Find {}", key);
    auto exists = props.find(key);
    if (exists != props.end())
        return exists->second;
    // log$("prop_optional_find", "{} not found", key);
    return {};
}

template <typename T>
optional<tuple<string, T>> prop_optional_find(const map<string, T> &props, function<bool(const T &elem)> test) {
    for (const auto &elem : props) {
        if (test(elem.second)) {
            return elem;
        }
    }
    return {};
}

string escape_json_string(const string &input) {
    return std::regex_replace(input, std::regex("\""), "\\\"");
}

Shortcuts::Shortcuts() : Loggable("🖇️ Shortcuts") {
}

string value_to_string(Value v);

string vdfmap_to_string(const shortcuts::Map &m) {
    auto out = fmt::memory_buffer();
    for (const auto &elem : m.v) {
        if (out.size() > 0)
            format_to(std::back_inserter(out), "{}", ", ");
        else
            format_to(std::back_inserter(out), "{} ", "{");
        format_to(std::back_inserter(out), "\"{}\" = {}", elem.first, value_to_string(elem.second));
    }
    if (out.size() == 0)
        format_to(std::back_inserter(out), "{}", "{");
    format_to(std::back_inserter(out), "{}", " }");
    // format_to(std::back_inserter(out), "{}", "\0");
    return string(out.data(), out.size());
}

string value_to_string(Value v) {
    if (holds_alternative<uint32_t>(v))
        return fmt::format("{}", get<uint32_t>(v));
    else if (holds_alternative<string>(v))
        return fmt::format("\"{}\"", get<string>(v));
    else if (holds_alternative<shortcuts::Map>(v))
        return vdfmap_to_string(get<shortcuts::Map>(v));

    return "Not supported";
}

bool Shortcuts::prop_is_uint32(const string &name) {
    return (get<1>(valid_props.at(name))) == PropsTypes::UInt32;
}

bool Shortcuts::prop_is_string(const string &name) {
    return (get<1>(valid_props.at(name))) == PropsTypes::String;
}

bool Shortcuts::prop_is_stringarr(const string &name) {
    return (get<1>(valid_props.at(name))) == PropsTypes::Strings;
}

string Shortcuts::resja(const string &json_stringified_array, bool emptystring_if_emptyarray) {
    auto parsed = json::parse(json_stringified_array);
    auto out = fmt::memory_buffer();
    format_to(std::back_inserter(out), "{}", "[");
    if (parsed.kind() != json::kind::array) {
        throw new runtime_error(fmt::format("Cannot restringify string \"{}\". It is not a valid json stringified array.", json_stringified_array));
    }

    auto pos = 0;
    for (const auto &elem : parsed.as_array()) {
        // log$("resja","elem: {}", elem.as_string().c_str());
        if (!elem.is_string()) {
            throw invalid_argument(fmt::format("Cannot cast json array value \"{}\" at position {} to string. It is not a string.", json_stringified_array, pos));
        }
        if (pos > 0)
            format_to(std::back_inserter(out), "{}", ",");
        format_to(std::back_inserter(out), "\"{}\"", escape_json_string(boost::json::value_to<std::string>(elem)));
        pos++;
    }

    if (pos == 0 && emptystring_if_emptyarray) {
        return "";
    }

    format_to(std::back_inserter(out), "{}", "]");

    return string(out.data(), out.size());
}

std::string to_utf8(std::wstring &wide_string) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(wide_string);
}

int Shortcuts::update_from_json_file(const fs::path &source) {

    std::wifstream wif(source);
    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    auto ws = wss.str();
    // string text(ws.begin(), ws.end());
    auto text = to_utf8(ws);
    // log("Loaded {}", text);
    try {
        return update_from_json(text);
    } catch (exception &e) {
        throw runtime_error(fmt::format("Cannot parse array from input file \"{}\":\n  {}", source.c_str(), e.what()));
    }
}

optional<string> some_json_string(const json::object &obj, const string &key) {
    auto exists = obj.find(key);
    try {
        if (exists != obj.end()) {
            return exists->value().as_string().c_str();
        }
    } catch (exception &e) {
        error$("some_json_string", "Find error for \"{}\". Expected string(type:5) got type:{}.", key, (uint8_t)exists->value().kind());
        error$("some_json_string:exception", "{}", e.what());
    }
    return {};
}

optional<uint32_t> some_json_u32(const json::object &obj, const string &key) {
    auto exists = obj.find(key);
    try {
        if (exists != obj.end()) {
            auto val = exists->value().as_int64();
            if (val < 0)
                throw runtime_error(fmt::format("Value `{}` must be convertible to uint32.", val));
            if (val >= std::numeric_limits<uint32_t>::max())
                throw runtime_error(fmt::format("Value `{}` must be convertible to uint32.", val));
            return (uint32_t)val;
        }
    } catch (exception &e) {
        error$("some_json_u32", "Find error for \"{}\". Expected int64(type:2) got type:{}.", key, (uint8_t)exists->value().kind());
        error$("some_json_u32:exception", "{}", e.what());
    }
    return {};
}
int Shortcuts::update_from_json(const string &text) {
    auto parsed = json::parse(text);
    if (parsed.kind() != json::kind::array) {
        throw runtime_error("Cannot parse array from input.");
    }

    auto count = 0;
    for (const auto &some_sc : parsed.as_array()) {
        const auto json_sc = some_sc.as_object();
        const auto index = some_json_u32(json_sc, "index");
        if (!index.has_value()) {
            throw runtime_error(fmt::format("Missing \"{}\" property at ShortcutArray[{}]", "index", count));
        }
        get_or_create(index.value(), [&json_sc, &count](bool isnew, Shortcut &sc) {
            for (auto &prop : valid_props) {
                switch (get<1>(prop.second)) {
                case PropsTypes::String:
                    if (auto val = some_json_string(json_sc, prop.first))
                        sc.props[prop.first] = val.value();
                    else
                        throw runtime_error(fmt::format("Missing \"{}\" property at ShortcutArray[{}]", prop.first, count));
                    break;
                case PropsTypes::UInt32: {
                    if (auto val = some_json_u32(json_sc, prop.first))
                        sc.props[prop.first] = val.value();
                    else
                        throw runtime_error(fmt::format("Missing \"{}\" property at ShortcutArray[{}]", prop.first, count));
                    break;
                }
                case PropsTypes::Strings:

                    break;
                }
            }
        });
        count++;
    }

    return count;
}
int Shortcuts::parse(const fs::path &source) {

    shortcuts.clear();

    ifstream file(source, ifstream::in | ios::binary | ios::ate);
    buffer_size = file.tellg();

    buffer_index = 0;
    file.seekg(0);

    // log("File openend \"{}\". Len: {}", source.string(), buffer_size);
    if (auto map = consume_map(file)) {
        // log("Map constructed. Size is: {}", (*map).v.size());

        // log("Dump:\n{}", vdfmap_to_string(*map));

        // convert to shortcut.

        auto mscs = map.value().v.find("shortcuts");
        if (mscs == map.value().v.end())
            throw invalid_argument("Missnk keyword \"shortcuts\" as first map key.");

        auto others = get<shortcuts::Map>(mscs->second);

        for (const auto &elem : others.v) {
            // log("In map first is {}", elem.first);
            auto index = (uint32_t)stoi(elem.first);
            shortcuts[index] = Shortcut(index, get<shortcuts::Map>(elem.second));
        }
    }

    return shortcuts.size();
}

void write_string(ofstream &out, const string &val) {
    out << val;
    out << '\0';
}
void write_byte(ofstream &out, uint8_t byte) {
    out << byte;
}

void write_u32(ofstream &out, uint32_t u32) {
    out.write((const char *)&u32, sizeof(uint32_t));
}

void Shortcuts::store_into(const string &destfile) {
    ofstream file(destfile, ofstream::out | ios::binary);
    write_byte(file, VdfMapItemType::Map);
    write_string(file, "shortcuts");
    for (const auto &elem : shortcuts) {
        elem.second.store_into(file);
    }
    write_byte(file, VdfMapItemType::MapEnd);

    write_byte(file, VdfMapItemType::MapEnd); //<----------- why last map end here?
}

void Shortcut::store_into(ofstream &out) const {
    write_byte(out, VdfMapItemType::Map);
    write_string(out, fmt::format("{}", get<uint32_t>(props.at("index"))));
    auto index = prop_optional_find(props, "index");
    // log("Write {}", get<string>(props.at("appname")));
    // Enforce write ordering
    for (auto order = 0; order < MAX_VALID_PROPS; order++) {
        auto propinfo = prop_optional_find<tuple<string, PropsTypes, uint8_t>>(valid_props, [&order](const auto &elem) {
                            return get<2>(elem) == order;
                        }).value();
        auto prop_name = get<0>(propinfo);

        auto exists = prop_optional_find(props, prop_name);
        if (!exists.has_value())
            throw runtime_error(fmt::format("Missing property \"{}\" on Shortcut[{}].", prop_name, index.has_value() ? fmt::format("{}", get<uint32_t>(index.value())) : "<no index>"));

        auto proptype = get<1>(propinfo);

        // log("Write {}", prop_name);
        switch (get<1>(proptype)) {
        case PropsTypes::UInt32:
            write_byte(out, VdfMapItemType::UInt32);
            break;
        case PropsTypes::String:
            write_byte(out, VdfMapItemType::String);
            break;
        case PropsTypes::Strings:
            write_byte(out, VdfMapItemType::Map);
            break;
        default:
            break;
        }

        // Write original prop name
        write_string(out, get<0>(proptype));

        switch (get<1>(proptype)) {
        case PropsTypes::UInt32:
            write_u32(out, get<uint32_t>(exists.value()));
            break;
        case PropsTypes::String:
            write_string(out, get<string>(exists.value()));
            break;
        case PropsTypes::Strings:
            // just tags!
            // write_string(out, Shortcuts::resja(get<string>(exists.value()), true));
            {
                auto parsed = json::parse(get<string>(exists.value()));

                if (parsed.kind() != json::kind::array) {
                    throw new runtime_error(fmt::format("Cannot restringify string \"{}\". It is not a valid json stringified array.", "tags"));
                }

                auto pos = 0;
                for (const auto &elem : parsed.as_array()) {
                    // log$("resja","elem: {}", elem.as_string().c_str());

                    write_byte(out, VdfMapItemType::String);
                    write_string(out, fmt::format("{}", pos));
                    write_string(out, fmt::format("{}", escape_json_string(boost::json::value_to<std::string>(elem))));
                    pos++;
                }

                write_byte(out, VdfMapItemType::MapEnd);
            }
            break;
        default:
            break;
        }
    }
    write_byte(out, VdfMapItemType::MapEnd);
}

void Shortcuts::foreach (function<bool(const Shortcut &sc)> f) {
    for (const auto &elem : shortcuts) {
        // log("In map first is {}", elem.first);
        if (!f(elem.second))
            break;
    }
}

bool Shortcuts::get_or_create(uint32_t index, function<void(bool isnew, Shortcut &sc)> f) {
    // log("Search shortcuts at {}", index);
    uint32_t maxidx = 0;
    for (auto &elem : shortcuts) {
        if (auto mval = prop_optional_find(elem.second.props, "index")) {
            auto idx = get<uint32_t>(mval.value());

            if (idx == index) {
                f(false, elem.second);
                return false; // no new Shortcut is created
            } else {
                maxidx = max(maxidx, idx);
            }
        }
    }
    // index not found, create newone

    auto sc = Shortcut();
    maxidx++;

    shortcuts[maxidx] = sc;

    sc.props["index"] = maxidx;
    sc.props["allow_desktop_config"] = 0U;
    sc.props["allow_overlay"] = 0U;
    sc.props["app_id"] = 0U;
    sc.props["app_name"] = "ERROR";
    sc.props["devkit"] = 0U;
    sc.props["devkit_game_id"] = "";
    sc.props["devkit_override_app_id"] = 0U;
    sc.props["exe"] = "ERROR";
    sc.props["flatpak_app_id"] = "";
    sc.props["icon"] = "";
    sc.props["is_hidden"] = 0U;
    sc.props["last_play_time"] = 0U;
    sc.props["launch_options"] = "";
    sc.props["open_vr"] = 0U;
    sc.props["shortcut_path"] = "";
    sc.props["start_dir"] = "";

    f(true, sc);

    return true;
}

// Shortcut *Shortcuts::at(uint32_t index)
// {
//     for (auto &elem : shortcuts)
//     {
//         if (auto mval = prop_optional_find(elem.second.props, "index"))
//         {
//             // auto fidx = get<uint32_t>(map_optional_find(elem.second.props, "index"));
//             if (auto idx = get<uint32_t>(mval.value()))
//             {
//                 if (idx == index){
//                     return &elem.second;
//                 }
//             }
//         }
//         //    if (boost::equals(elem.second.props["index"], index))
//     }
// }

optional<tuple<string, Value>> Shortcuts::consume_map_item(ifstream &file) {
    if (eof()) {
        return {};
    }
    if (auto btype = consume_byte(file)) {
        string name;
        switch (btype.value()) {
        case VdfMapItemType::MapEnd:
            return {};
        case VdfMapItemType::Map:
        case VdfMapItemType::String:
        case VdfMapItemType::UInt32: {
            auto name_2 = consume_string(file);
            if (!name_2.has_value()) {
                error("[{:x}]  - Expected string at {:x}.", buffer_index, btype.value());
                return {};
            }
            name = name_2.value();

            switch (btype.value()) {
            case VdfMapItemType::Map:
                if (auto val = consume_map(file))
                    return make_optional(make_tuple(name, val.value()));
                break;
            case VdfMapItemType::String:
                if (auto val = consume_string(file))
                    return make_optional(make_tuple(name, val.value()));
                break;
            case VdfMapItemType::UInt32:
                if (auto val = consume_uint32(file))
                    return make_optional(make_tuple(name, val.value()));
                break;
            }
        } break;

        default:
            error("[{:x}]  - type {:x} is invalid.", buffer_index, btype.value());
            return {};
        }
    }

    error("[{:x}]  - expected byte but EOF.", buffer_index);
    return {};
}
optional<string> Shortcuts::consume_string(ifstream &file) {
    if (eof())
        return {};

    string word;

    while (!eof()) {
        if (auto ch = consume_byte(file)) {
            if (ch == 0)
                break;
            word.push_back(ch.value());

            if (ch >= 128) {
            }
        } else if (word.size() > 0) {
            break;
        } else {
            return {};
        }
    };
    // str.push_back()

    // log("[{:x}] Word readen: {}",buffer_index, word);

    return word;
}
optional<uint32_t> Shortcuts::consume_uint32(ifstream &file) {
    if (eof())
        return {};
    uint32_t b;

    // log("[{:x}]/[{:x}] UInt32 reading", buffer_index, file.tellg());
    file.read(reinterpret_cast<char *>(&b), sizeof(b));

    // log("[{:x}] UInt32 pre read: {}", buffer_index, b);
    buffer_index += sizeof(b);

    return b;
}
optional<uint8_t> Shortcuts::consume_byte(ifstream &file) {
    if (eof())
        return {};
    uint8_t b;
    file.read((char *)&b, 1);
    buffer_index++;
    return b;
}
optional<uint8_t> Shortcuts::peek_byte(ifstream &file) {
    if (eof())
        return {};
    uint8_t b;
    file.read((char *)&b, 1);
    file.seekg(buffer_index); // returns back
    return b;
}
bool Shortcuts::eof() {
    return buffer_index >= buffer_size;
}
optional<shortcuts::Map> Shortcuts::consume_map(ifstream &file) {
    VdfMap map;

    while (!eof()) {
        if (auto item = consume_map_item(file)) {
            boost::algorithm::to_lower(std::get<0>(*item));
            map[std::get<0>(*item)] = std::get<1>(*item);
        } else
            break;
    }

    return make_optional<shortcuts::Map>(Map{.v = map});
}

Shortcut::Shortcut() : Loggable("🔗 Shortcut") {
}

#define copy_shortcut_param$(name, defvalue)                                                                                                     \
    switch (get<1>(valid_props.at(#name))) {                                                                                                     \
    case PropsTypes::UInt32:                                                                                                                     \
        props[#name] = get<uint32_t>(map_optional_find(map, boost::algorithm::to_lower_copy(get<0>(valid_props.at(#name)))).value_or(defvalue)); \
        break;                                                                                                                                   \
    case PropsTypes::String:                                                                                                                     \
        props[#name] = get<string>(map_optional_find(map, boost::algorithm::to_lower_copy(get<0>(valid_props.at(#name)))).value_or(defvalue));   \
        break;                                                                                                                                   \
    default:                                                                                                                                     \
        throw runtime_error("Not supported");                                                                                                    \
    }

optional<Value>
map_optional_find(const shortcuts::Map &map, const string &index) {
    auto exists = map.v.find(index);
    if (exists != map.v.end())
        return exists->second;
    return {};
}

Shortcut::Shortcut(uint32_t index, const shortcuts::Map &map) : Shortcut() {
    // log("Dump map: {}", vdfmap_to_string(map));
    props["index"] = index;
    copy_shortcut_param$(allow_desktop_config, 0U);
    copy_shortcut_param$(allow_overlay, 0U);
    copy_shortcut_param$(app_id, 0U);
    copy_shortcut_param$(app_name, "ERROR");
    copy_shortcut_param$(devkit, 0U);
    copy_shortcut_param$(devkit_game_id, "");
    copy_shortcut_param$(devkit_override_app_id, 0U);
    copy_shortcut_param$(exe, "ERROR");
    copy_shortcut_param$(flatpak_app_id, "");
    copy_shortcut_param$(icon, "");
    copy_shortcut_param$(is_hidden, 0U);
    copy_shortcut_param$(last_play_time, 0U);
    copy_shortcut_param$(launch_options, "");
    copy_shortcut_param$(open_vr, 0U);
    copy_shortcut_param$(shortcut_path, "");
    copy_shortcut_param$(start_dir, "");

    if (auto mtags = map_optional_find(map, "tags")) {
        auto out = fmt::memory_buffer();
        for (const auto &elem : get<shortcuts::Map>(mtags.value()).v) {
            if (out.size() != 0)
                format_to(std::back_inserter(out), "{}", ", ");
            else
                format_to(std::back_inserter(out), "{}", "[");
            format_to(std::back_inserter(out), "\"{}\"", escape_json_string(get<String>(elem.second)));
        }
        if (out.size() == 0)
            format_to(std::back_inserter(out), "{}", "[");
        format_to(std::back_inserter(out), "{}", "]");
        props["tags"] = string(out.data(), out.size());
    }
}

PropsValidReturns Shortcuts::is_prop_valid(const string &key, const string &value, bool ignore_value_check) {
    auto key_is_ok = false;
    auto logname = "🔗 Shortcut";
    PropsTypes ptype = PropsTypes::String;
    for (const auto &pvalid : valid_props) {
        if (boost::iequals(pvalid.first, key)) {
            key_is_ok = true;
            ptype = get<1>(pvalid.second);

            break;
        }
    }
    if (!key_is_ok) {
        return PropsValidReturns::InvalidKey;
    }

    if (ignore_value_check)
        return PropsValidReturns::Ok;

    if (ptype == PropsTypes::UInt32) {
        optional<uint32_t> num = std::stoi(value);
        if (!num.has_value()) {
            error$(logname, "Unable to convert from {} to UInt32.", value);
            return PropsValidReturns::InvalidValue;
        }

        if (boost::iequals("app_id", key) && num.value() <= 0) {
            error$(logname, "Value cannot be <=0 for \"{}\".", "app_id");
            return PropsValidReturns::InvalidValue;
        }
    } else if (ptype == PropsTypes::String) {
        if (boost::iequals("app_name", key) && value.empty()) {
            error$(logname, "Value cannot be empty for \"{}\".", "app_name");
            return PropsValidReturns::InvalidValue;
        }

        return PropsValidReturns::Ok;
    } else if (ptype == PropsTypes::Strings) {
        try {
            resja(value);
        } catch (exception &e) {
            return PropsValidReturns::InvalidArrayValue;
        }
        return PropsValidReturns::Ok;
    }

    return PropsValidReturns::Ok;
}