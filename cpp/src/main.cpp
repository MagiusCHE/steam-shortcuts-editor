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
    hidden.add_options()(switchname, po::value(&format_switches[switchname]), "Shows " #bettername " with specified format [default: " defvalue "] [possible values: None, Plain]")

#define set_format_switches_ex$(switchname, bettername, defvalue) \
    format_switches["" switchname] = "" defvalue;                 \
    hidden.add_options()(switchname, po::value(&format_switches[switchname]), bettername " [default: " defvalue "] [possible values: None, Plain]")

#define test$(color) "\u001B[38;5;" color

#define color_string$(color) \
    "\u001B[38;5;" #color "m"

#define color_clear_string$ \
    "\u001b[0m"

// https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
#define C_TITLE color_string$(130)
#define C_OPT color_string$(107)
#define C_LS color_clear_string$

int main(int argc, char *argv[])
{
    logging::update_debug_src_root_path(__FILE__);
    // log$("main", "{}", "Start");

    try
    {
        string ifile;
        string command;
        string separator = " ";
        unordered_map<string, string>
            format_switches;

        auto help_main =
            "VDF Shortcuts Editor for Steam Client\n\n" C_TITLE "USAGE:\n" C_LS
            "    steam-shortcuts-editor <SHORTCUTS_PATH> <SUBCOMMAND>\n\n" C_TITLE "ARGS:\n" C_OPT "    <SHORTCUTS_PATH>" C_LS "    Path to \"shortcuts.vdf\"\n\n" C_TITLE "SUBCOMMANDS:\n" C_OPT "    help" C_LS "       Print this message or the help of the given subcommand(s)\n" C_OPT "    version" C_LS "    Print version information\n" C_LS;

        po::options_description hidden("Hidden options");
        // desc.add_options()("help,h", "Print help information");
        // desc.add_options()("list", "List entries summary info");
        hidden.add_options()("input", po::value(&ifile), "");
        hidden.add_options()("command", po::value(&command), "");
        po::positional_options_description positional;
        positional.add("input", 1);
        positional.add("command", -1);

        hidden.add_options()("separator", po::value(&separator), "Table output columns separator");
        hidden.add_options()("keys", "Show key for each value in table output");

        set_format_switches$("index", "Index", "None");
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
        po::store(po::command_line_parser(argc, argv).options(hidden).positional(positional).run(), vm);
        po::notify(vm);

        fmt::print("Input: {}\n", ifile);
        fmt::print("Command: {}\n", command);

        if (boost::iequals(ifile, "version"))
        {
            // print spcific command
            fmt::print("{} {}\n\n", "steam-shortcuts-editor", "0.1.0");
            return 0;
        }

        if (!vm.count("command") || boost::iequals(command, "help"))
        {
            cout << help_main << "\n\n";
            return 0;
        }

        if (boost::iequals(ifile, "help"))
        {
            if (boost::iequals(command, "list"))
            {
                // print spcific command
                fmt::print("List entries summary info\n\n" C_TITLE "USAGE:\n" C_LS
                           "    steam-shortcuts-editor <SHORTCUTS_PATH> list <ARGS>\n\n" C_TITLE "ARGS:\n" C_LS);
                for (const auto &elem : format_switches)
                {
                    fmt::print("    " C_OPT "--{} <format>\n" C_LS "       {}\n\n", elem.first, hidden.find(elem.first,false,true).description());
                }
                return 0;
            }
            else
            {
                throw invalid_argument("Help command must be followed by a subcommand.");
            }
        }

        if (ifile.empty())
            throw invalid_argument("Missing input shortcuts file.");

        const fs::path vdf(ifile);
        if (!fs::exists(vdf))
        {
            throw runtime_error(fmt::format("Shortcuts file not exists at: \"{}\"", vdf.string()));
        }

        auto vdf_fullpath = fs::canonical(vdf);

        auto scs = shortcuts::Shortcuts(vdf_fullpath);
        // All Shortcuts will be created
        scs.parse();

        if (boost::iequals(command, "list"))
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
    catch (const po::multiple_occurrences &e)
    {
        error$("ERROR", "{}", "Cannot specify more than 2 positional arguments: <SHORTCUTS_PATH> <SUBCOMMAND>.");
        error$("main", "{}", "Try use --help.");
        error$("main", "{}", "Program aborted.");
    }
    catch (const exception &e)
    {
        error$("ERROR", "{}", /*typeid(e).name(), */ e.what());
        error$("main", "{}", "Program aborted.");
    }
}