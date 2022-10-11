/*
 * Copyright (c) 2022, Magius(CHE)
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 * Read the LICENSE file for more details.
 *
 * @author: Magius(CHE) - magiusche@magius.it
 */

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

#define bold_string$ \
    "\u001B[1m"

#define underline_string$ \
    "\u001B[4m"

#define clear_string$ \
    "\u001B[0m"

// https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
//#define C_TITLE color_string$(130)
#define C_TITLE bold_string$ underline_string$
//#define C_OPT color_string$(107)
#define C_OPT bold_string$
#define C_LS clear_string$

#ifndef PROJECT_NAME
#define PROJECT_NAME "steam-shortcuts-editor"
#endif
#ifndef PROJECT_VERSION
#define PROJECT_VERSION "custom"
#endif
#ifndef PROJECT_AUTHOR_NAME
#define PROJECT_AUTHOR_NAME "Magius(CHE)"
#endif
#ifndef PROJECT_AUTHOR_EMAIL
#define PROJECT_AUTHOR_NAME "magiusche@magius.it"
#endif
#ifndef PROJECT_HOMEPAGE
#define PROJECT_HOMEPAGE "https://github.com/magiusche/steam-shortcuts-editor"
#endif

int main(int argc, char *argv[])
{
    logging::update_debug_src_root_path(__FILE__);
    // log$("main", "{}", "Start");

    try
    {
        string argument_01;
        string command;
        string separator = " ";
        unordered_map<string, string>
            format_switches;

        auto help_main =
            "VDF Shortcuts Editor for Steam Client\n\n" C_TITLE "Usage:" C_LS
            " steam-shortcuts-editor <COMMAND>\n\n" C_TITLE "Commands:" C_LS "\n"
            "    " C_OPT "list" C_LS "     List entries summary info\n"
            "    " C_OPT "version" C_LS "  Print version information\n"
            "    " C_OPT "help" C_LS "     Print this message or the help of the given subcommand(s)\n"
            "\n" C_TITLE "Options:" C_LS "\n"
            "    " C_OPT "-h, --help" C_LS "  Print this message.\n";

        po::options_description hidden("Hidden options");
        // desc.add_options()("help,h", "Print help information");
        // desc.add_options()("list", "List entries summary info");
        hidden.add_options()("command", po::value(&command), "");
        hidden.add_options()("argument_01", po::value(&argument_01), "");
        po::positional_options_description positional;
        positional.add("command", 1);
        positional.add("argument_01", -1);

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

#if DEBUG==1
        fmt::print("argument_01: {}\n", argument_01);
        fmt::print("Command: {}\n", command);
#endif

        if (boost::iequals(argument_01, "version"))
        {
            // print spcific command
            fmt::print("{} {} by {} - {}, {}\n\n", PROJECT_NAME, PROJECT_VERSION, PROJECT_AUTHOR_NAME, PROJECT_AUTHOR_EMAIL, PROJECT_HOMEPAGE);
            return 0;
        }

        if (boost::iequals(command, "help") && !vm.count("argument_01"))
        {
            cout << help_main << "\n\n";
            return 0;
        }

        if (boost::iequals(command, "help"))
        {
            if (boost::iequals(argument_01, "list"))
            {
                // print spcific command
                fmt::print("List entries summary info\n\n" C_TITLE "Usage:\n" C_LS " steam-shortcuts-editor list [OPTIONS] <SHORTCUTS_PATH>\n\n"
                           C_TITLE "Arguments:\n" C_LS
                           "  <SHORTCUTS_PATH>  Path to \"shortcuts.vdf\"\n\n"
                           C_TITLE "Options:\n" C_LS);

                fmt::print("    " C_OPT "--separator <SEPARATOR>\n" C_LS "       Table output columns separator [default: \" \"]\n\n");
                fmt::print("    " C_OPT "--keys\n" C_LS "       Show key for each value in table output\n\n");
                for (const auto &elem : format_switches)
                {
                    fmt::print("    " C_OPT "--{} <format>\n" C_LS "       {}\n\n", elem.first, hidden.find(elem.first, false, true).description());
                }
                return 0;
            }
            else
            {
                throw invalid_argument("Help command must be followed by a command to inspect.");
            }
        }

        if (argument_01.empty())
            throw invalid_argument("Missing input shortcuts file. Check the usage.");

        const fs::path vdf(argument_01);
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