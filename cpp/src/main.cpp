#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#define FMT_HEADER_ONLY
#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "./shortcuts.hpp"
#include "./logging.hpp"
#include <chrono>
#include <thread>

using namespace std;

namespace fs = filesystem;

namespace po = boost::program_options;

#define set_format_switches$(switchname, bettername, defvalue) \
    format_switches["" switchname] = "" defvalue;              \
    desc.add_options()(switchname, po::value(&format_switches[switchname]), "Shows " #bettername " with specified format [default: " defvalue "] [possible values: None, Plain]")

#define set_format_switches_ex$(switchname, bettername, defvalue) \
    format_switches["" switchname] = "" defvalue;                 \
    desc.add_options()(switchname, po::value(&format_switches[switchname]), bettername " [default: " defvalue "] [possible values: None, Plain]")

int main(int argc, char *argv[])
{
    logging::update_debug_src_root_path(__FILE__);
    // log$("main", "{}", "Start");

    try
    {
        string ifile;
        string separator = " ";
        map<string, string>
            format_switches;

        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("list", "list all shortcuts");
        desc.add_options()("input,i", po::value(&ifile), "pathname of shortcuts.vdf");
        desc.add_options()("separator", po::value(&separator), "Table output columns separator");
        desc.add_options()("keys", "Show key for each value in table output");

        set_format_switches$("allow_desktop_config", "AllowDesktopConfig", "None");
        set_format_switches$("allow_overlay", "AllowOverlay", "None");
        set_format_switches$("appid", "AppId", "Plain");
        set_format_switches$("appname", "AppName", "Plain");
        set_format_switches$("devkit_game_id", "DevkitGameId", "None");
        set_format_switches$("devkit_override_app_id", "DevkitOverrideAppId", "None");
        set_format_switches$("devkit", "Devkit", "None");
        set_format_switches$("exe", "Exe", "None");
        set_format_switches$("flatpak_app_id", "FlatpakAppId", "None");
        set_format_switches$("icon", "Icon", "None");
        set_format_switches$("is_hidden", "IsHidden", "None");
        set_format_switches$("last_play_time_fmt", "LastPlayTime in YYYY/MM/DD, hh:mm:ss (Localtime)", "None");
        set_format_switches$("last_play_time_iso", "LastPlayTime in ISO", "None");
        set_format_switches$("last_play_time_utc", "LastPlayTime in YYYY/MM/DD, hh:mm:ss UTC", "None");
        set_format_switches$("last_play_time", "LastPlayTime", "None");
        set_format_switches$("launch_options", "LaunchOptions", "None");
        set_format_switches$("open_vr", "OpenVR", "None");
        set_format_switches$("shortcut_path", "ShortcutPath", "None");
        set_format_switches$("start_dir", "StartDir", "None");
        set_format_switches$("tags", "Tags", "None");

        set_format_switches_ex$("all", "Override all columns format with the specified one. None = ignored.", "None");
        // desc.add_options()("list", "list all shortcuts");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            cout << desc << "\n";
            return 0;
        }

        if (ifile.empty())
            throw invalid_argument("Missing input shortcuts file.");

        const fs::path vdf(ifile);
        if (!fs::exists(vdf))
        {
            throw runtime_error(fmt::format("Shortcuts file not exists at: \"{}\"", vdf.string()));
        }

        auto vdf_fullpath = fs::canonical(vdf);

        // log$("main", "Analyze: {}", vdf_fullpath.string());

        auto hascommand = vm.count("list");

        if (!hascommand)
        {
            cout << desc << "\n";
            return 0;
        }

        // log$("main","Load file {}",vm.at())

        auto scs = shortcuts::Shortcuts(vdf_fullpath);
        // All Shortcuts will be created
        scs.parse();

        if (vm.count("list"))
        {
            const auto all_format = format_switches["all"];
            const auto showkeys = vm.count("keys");
            auto out = fmt::memory_buffer();
            scs.foreach ([&format_switches, &out, &all_format, &separator, &showkeys](const shortcuts::Shortcut &sc)
                         {
                            //log$("main", "Cycle {}",sc.index);
                            auto first = true;
                            for (const auto &elem : format_switches)
                            {
                                if (elem.first == "all")
                                    continue;
                                // log$("main", " - sw {} = {}", elem.first,elem.second);
                                if (boost::iequals(elem.second, "Plain") || boost::iequals(all_format, "Plain"))
                                {
                                    if (!first)
                                        format_to(std::back_inserter(out), "{}", separator);
                                    first = false;
                                    if (showkeys)
                                        format_to(std::back_inserter(out), "{} = ", elem.first);
                                    auto val = sc.props.find(elem.first);
                                    if (val != sc.props.end())
                                    {
                                        if (auto st = get_if<string>(&val->second))
                                        {
                                            // log$("main", "   - {}", *st);
                                            format_to(std::back_inserter(out), (val->first == "tags") ? "{}" : "\"{}\"", *st);
                                        }
                                        else if (auto st = get_if<uint32_t>(&val->second))
                                        {
                                            // log$("main", "   - {}", *st);
                                            format_to(std::back_inserter(out), "{}", *st);
                                        }
                                    }
                                    else
                                    {
                                        // log$("main", "   - {}", "");
                                        format_to(std::back_inserter(out), "{}", "<missing>");
                                    }
                                    // if (get<uint32_t>(val))
                                    // format_to(std::back_inserter(out), "{}", );
                                }
                    
                }
                format_to(std::back_inserter(out), "{}", "\n");
                return true; });

            fmt::print("{}", string(out.data(), out.size()));
        }
    }
    catch (const exception &e)
    {
        error$("ERROR", "{}", e.what());
        error$("main", "{}", "Try use --help.");
        error$("main", "{}", "Program aborted.");
    }
}