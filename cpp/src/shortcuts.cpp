#include "./shortcuts.hpp"
#include <fstream> // std::ifstream
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace shortcuts;
namespace fs = filesystem;

enum VdfMapItemType : uint8_t
{
    Map = 0x00,
    String = 0x01,
    UInt32 = 0x02,
    MapEnd = 0x08,
};

Shortcuts::Shortcuts() : Loggable("🖇️ Shortcuts")
{
}

Shortcuts::Shortcuts(string source) : Shortcuts(fs::path(source)) {}

Shortcuts::Shortcuts(const fs::path source) : Shortcuts()
{
    source_file = source;
}

string value_to_string(Value v);

string vdfmap_to_string(const shortcuts::Map &m)
{
    auto out = fmt::memory_buffer();
    for (const auto &elem : m.v)
    {
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

string value_to_string(Value v)
{
    if (holds_alternative<uint32_t>(v))
        return fmt::format("{}", get<uint32_t>(v));
    else if (holds_alternative<string>(v))
        return fmt::format("\"{}\"", get<string>(v));
    else if (holds_alternative<shortcuts::Map>(v))
        return vdfmap_to_string(get<shortcuts::Map>(v));

    return "Not supported";
}

int Shortcuts::parse()
{
    shortcuts.clear();

    ifstream file(source_file, ifstream::in | ios::binary | ios::ate);
    buffer_size = file.tellg();

    buffer_index = 0;
    file.seekg(0);

    // log("File openend \"{}\". Len: {}", source_file.string(), buffer_size);
    if (auto map = consume_map(file))
    {
        // log("Map constructed. Size is: {}", (*map).v.size());

        // log("Dump:\n{}", vdfmap_to_string(*map));

        // convert to shortcut.

        auto mscs = map.value().v.find("shortcuts");
        if (mscs == map.value().v.end())
            throw invalid_argument("Missnk keyword \"shortcuts\" as first map key.");

        auto others = get<shortcuts::Map>(mscs->second);

        for (const auto &elem : others.v)
        {
            // log("In map first is {}", elem.first);
            auto index = (uint32_t)stoi(elem.first);
            shortcuts[index] = Shortcut(index, get<shortcuts::Map>(elem.second));
        }
    }

    return shortcuts.size();
}

void Shortcuts::foreach (function<bool(const Shortcut &sc)> f)
{
    for (const auto &elem : shortcuts)
    {
        // log("In map first is {}", elem.first);
        if (!f(elem.second))
            break;
    }
}

optional<tuple<string, Value>> Shortcuts::consume_map_item(ifstream &file)
{
    if (eof())
    {
        return {};
    }
    if (auto btype = consume_byte(file))
    {

        string name;
        switch (btype.value())
        {
        case VdfMapItemType::MapEnd:
            return {};
        case VdfMapItemType::Map:
        case VdfMapItemType::String:
        case VdfMapItemType::UInt32:
        {
            auto name_2 = consume_string(file);
            if (!name_2.has_value())
            {
                error("[{:x}]  - Expected string at {:x}.", buffer_index, btype.value());
                return {};
            }
            name = name_2.value();

            switch (btype.value())
            {
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
        }
        break;

        default:
            error("[{:x}]  - type {:x} is invalid.", buffer_index, btype.value());
            return {};
        }
    }

    error("[{:x}]  - expected byte but EOF.", buffer_index);
    return {};
}
optional<string> Shortcuts::consume_string(ifstream &file)
{
    if (eof())
        return {};

    string word;

    while (!eof())
    {
        if (auto ch = consume_byte(file))
        {
            if (ch == 0)
                break;
            word.push_back(ch.value());

            if (ch >= 128)
            {
            }
        }
        else if (word.size() > 0)
        {
            break;
        }
        else
        {
            return {};
        }
    };
    // str.push_back()

    // log("[{:x}] Word readen: {}",buffer_index, word);

    return word;
}
optional<uint32_t> Shortcuts::consume_uint32(ifstream &file)
{
    if (eof())
        return {};
    uint32_t b;

    // log("[{:x}]/[{:x}] UInt32 reading", buffer_index, file.tellg());
    file.read(reinterpret_cast<char *>(&b), sizeof(b));

    // log("[{:x}] UInt32 pre read: {}", buffer_index, b);
    buffer_index += sizeof(b);

    return b;
}
optional<uint8_t> Shortcuts::consume_byte(ifstream &file)
{
    if (eof())
        return {};
    uint8_t b;
    file.read((char *)&b, 1);
    buffer_index++;
    return b;
}
optional<uint8_t> Shortcuts::peek_byte(ifstream &file)
{
    if (eof())
        return {};
    uint8_t b;
    file.read((char *)&b, 1);
    file.seekg(buffer_index); // returns back
    return b;
}
bool Shortcuts::eof()
{
    return buffer_index >= buffer_size;
}
optional<shortcuts::Map> Shortcuts::consume_map(ifstream &file)
{
    VdfMap map;

    while (!eof())
    {
        if (auto item = consume_map_item(file))
        {
            boost::algorithm::to_lower(std::get<0>(*item));
            map[std::get<0>(*item)] = std::get<1>(*item);
        }
        else
            break;
    }

    return make_optional<shortcuts::Map>(Map{.v = map});
}

Shortcut::Shortcut() : Loggable("🔗 Shortcut")
{
}

#define copy_shortcut_param$(name, key, type, defvalue) \
    props[#name] = get<type>(map_optional_find(map, #key).value_or((type)defvalue))

optional<Value> map_optional_find(const shortcuts::Map &map, const string &index)
{

    auto exists = map.v.find(index);
    if (exists != map.v.end())
        return exists->second;
    return {};
}

Shortcut::Shortcut(uint32_t index, const shortcuts::Map &map) : Shortcut()
{
    //log("Dump map: {}", vdfmap_to_string(map));
    props["index"] = index;
    copy_shortcut_param$(allow_desktop_config, allowdesktopconfig, uint32_t, 0);
    copy_shortcut_param$(allow_overlay, allowoverlay, uint32_t, 0);
    copy_shortcut_param$(appid, appid, uint32_t, 0);
    copy_shortcut_param$(appname, appname, string, "ERROR");
    copy_shortcut_param$(devkit_game_id, devkitgameid, string, "");
    copy_shortcut_param$(devkit_override_app_id, devkitoverrideappid, uint32_t, 0);
    copy_shortcut_param$(devkit, devkit, uint32_t, 0);
    copy_shortcut_param$(exe, exe, string, "ERROR");
    copy_shortcut_param$(flatpak_app_id, flatpakappid, string, "");
    copy_shortcut_param$(icon, icon, string, "");
    copy_shortcut_param$(is_hidden, ishidden, uint32_t, 0);
    copy_shortcut_param$(last_play_time, lastplaytime, uint32_t, 0);
    copy_shortcut_param$(launch_options, launchoptions, string, "");
    copy_shortcut_param$(open_vr, openvr, uint32_t, 0);
    copy_shortcut_param$(shortcut_path, shortcutpath, string, "");
    copy_shortcut_param$(start_dir, startdir, string, "");

    if (auto mtags = map_optional_find(map, "tags"))
    {
        auto out = fmt::memory_buffer();
        for (const auto &elem : get<shortcuts::Map>(mtags.value()).v)
        {
            if (out.size() != 0)
                format_to(std::back_inserter(out), "{}", ", ");
            else
                format_to(std::back_inserter(out), "{}", "[");
            format_to(std::back_inserter(out), "\"{}\"", get<String>(elem.second));
        }
        if (out.size() == 0)
            format_to(std::back_inserter(out), "{}", "[");
        format_to(std::back_inserter(out), "{}", "]");
        props["tags"] = string(out.data(), out.size());
    }

    /*log("{}","Dump props:");
    for (const auto &elem : props)
    {
        if (auto sc = get_if<uint32_t>(&elem.second))
            log("- {} = {}", elem.first, *sc);
        else if (auto sc = get_if<string>(&elem.second))
            log("- {} = {}", elem.first, *sc);
    }*/
}