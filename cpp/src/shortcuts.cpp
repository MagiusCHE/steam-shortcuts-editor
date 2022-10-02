#include "./shortcuts.hpp"
#include <fstream> // std::ifstream

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

string vdfpam_to_string(shortcuts::Map m)
{
    auto out = fmt::memory_buffer();
    for (const auto &elem : m.v)
    {
        format_to(std::back_inserter(out), "{} = {}", elem.first, value_to_string(elem.second));
    }
    // format_to(std::back_inserter(out), "{}", "\0");
    return string(out.data(), out.size());
}

string value_to_string(Value v)
{
    if (holds_alternative<uint32_t>(v))
        return fmt::format("{}", get<uint32_t>(v));
    else if (holds_alternative<string>(v))
        return get<string>(v);
    else if (holds_alternative<shortcuts::Map>(v))
        return vdfpam_to_string(get<shortcuts::Map>(v));

    return "Not supported";
}

int Shortcuts::parse()
{
    std::ifstream file(source_file, std::ifstream::in | ios::binary | ios::ate);
    buffer_size = file.tellg();
    buffer_index = 0;
    file.seekg(0);
    log("File openend \"{}\". Len: {}", source_file.string(), buffer_size);
    if (auto map = consume_map())
    {

        log("Map constructed. Size is: {}", (*map).v.size());

        log("Dump:\n{}", vdfpam_to_string(*map));
    }

    return 0;
}

optional<tuple<string, Value>> Shortcuts::consume_map_item()
{
    return {};
}
bool Shortcuts::eof()
{
    return buffer_index >= buffer_size;
}
optional<shortcuts::Map> Shortcuts::consume_map()
{
    VdfMap map;

    while (!eof())
    {
        if (auto item = consume_map_item())
            map[std::get<0>(*item)] = std::get<1>(*item);
        break;
    }

    map["Test key"] = Value("test passed");
    return make_optional<shortcuts::Map>(Map{.v = map});
}

Shortcut::Shortcut() : Loggable("🔗 Shortcut")
{
}
