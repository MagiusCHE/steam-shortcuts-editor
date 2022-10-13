/*
 * Copyright (c) 2022, Magius(CHE)
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 * Read the LICENSE file for more details.
 *
 * @author: Magius(CHE) - magiusche@magius.it
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#define FMT_HEADER_ONLY
#include <boost/algorithm/string/predicate.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <regex>
#include <thread>

#include "./logging.hpp"
#include "./shortcuts.hpp"

using namespace std;
using namespace shortcuts;

namespace fs = filesystem;

namespace po = boost::program_options;

#define set_format_switches$(switchname, bettername, defvalue) \
    format_switches["" switchname] = "" defvalue;              \
    hidden.add_options()(std::regex_replace(switchname, std::regex("_"), "-").c_str(), po::value(&format_switches[switchname]), "Shows " bettername " with specified format [default: " defvalue "] [possible values: None, Plain]")

#define set_format_switches_ex$(switchname, bettername, defvalue) \
    format_switches["" switchname] = "" defvalue;                 \
    hidden.add_options()(std::regex_replace(switchname, std::regex("_"), "-").c_str(), po::value(&format_switches[switchname]), bettername " [default: " defvalue "] [possible values: None, Plain]")

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

struct EditOptions {
    string json_path;
    string key;
    string val;
    uint32_t idx;
    string out;
} edit_options;

void show_list(const unordered_map<string, string> &format_switches, Shortcuts &scs, const string &separator, bool showkeys, bool json);

int main(int argc, char *argv[]) {
    logging::update_debug_src_root_path(__FILE__);
    // log$("main", "{}", "Start");

    try {
        string argument_01;
        string command;
        string separator = " ";
        unordered_map<string, string>
            format_switches;

        auto help_main =
            "VDF Shortcuts Editor for Steam Client\n\n" C_TITLE "Usage:" C_LS
            " steam-shortcuts-editor <COMMAND>\n\n" C_TITLE "Commands:" C_LS
            "\n"
            "    " C_OPT "list" C_LS
            "     List entries summary info\n"
            "    " C_OPT "edit" C_LS
            "     Update entries structure recreating .vdf shortcuts file\n"
            "    " C_OPT "version" C_LS
            "  Print version information\n"
            "    " C_OPT "help" C_LS
            "     Print this message or the help of the given subcommand(s)\n"
            "\n" C_TITLE "Options:" C_LS
            "\n"
            "    " C_OPT "-h, --help" C_LS "  Print this message.\n";

        po::options_description hidden("Hidden options");
        // desc.add_options()("help,h", "Print help information");
        // desc.add_options()("list", "List entries summary info");
        hidden.add_options()("command", po::value(&command), "");
        hidden.add_options()("argument_01", po::value(&argument_01), "");
        po::positional_options_description positional;
        positional.add("command", 1);
        positional.add("argument_01", -1);

        hidden.add_options()("separator", po::value(&separator), "");
        hidden.add_options()("keys", "");

        set_format_switches$("index", "Index", "None");
        set_format_switches$("allow_desktop_config", "AllowDesktopConfig", "None");
        set_format_switches$("allow_overlay", "AllowOverlay", "None");
        set_format_switches$("app_id", "AppId", "Plain");
        set_format_switches$("app_name", "AppName", "Plain");
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

        hidden.add_options()("json", "");
        // desc.add_options()("list", "list all shortcuts");

        hidden.add_options()("idx", po::value(&edit_options.idx));
        hidden.add_options()("key", po::value(&edit_options.key));
        hidden.add_options()("val", po::value(&edit_options.val));
        hidden.add_options()("json-path", po::value(&edit_options.json_path));
        hidden.add_options()("out", po::value(&edit_options.out));
        hidden.add_options()("force", "");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(hidden).positional(positional).run(), vm);
        po::notify(vm);

#if DEBUG
        fmt::print("Command: {}\n", command);
        fmt::print("argument_01: {}\n", argument_01);
#endif

        if (boost::iequals(argument_01, "version")) {
            // print spcific command
            fmt::print("{} {} by {} - {}, {}\n\n", PROJECT_NAME, PROJECT_VERSION, PROJECT_AUTHOR_NAME, PROJECT_AUTHOR_EMAIL, PROJECT_HOMEPAGE);
            return 0;
        }

        if (boost::iequals(command, "help") && !vm.count("argument_01")) {
            cout << help_main << "\n\n";
            return 0;
        }

        if (boost::iequals(command, "help")) {
            if (boost::iequals(argument_01, "list")) {
                // print spcific command
                fmt::print("List entries summary info\n\n" C_TITLE "Usage:\n" C_LS " steam-shortcuts-editor list [OPTIONS] <SHORTCUTS_PATH>\n\n" C_TITLE "Arguments:\n" C_LS
                           "  <SHORTCUTS_PATH>  Path to \"shortcuts.vdf\"\n\n" C_TITLE "Options:\n" C_LS);

                fmt::print("      " C_OPT "--separator <SEPARATOR>\n" C_LS "          Table output columns separator [default: \" \"]\n");
                fmt::print("      " C_OPT "--keys\n" C_LS "          Show key for each value in table output\n\n");
                fmt::print("      " C_OPT "--json\n" C_LS "          Export list in JSON format. This will ignore \"--separator\", \"--keys\", \"--last-play-time-*\".\n\n");
                for (const auto &elem : format_switches) {
                    auto repl = std::regex_replace(elem.first, std::regex("_"), "-");
                    fmt::print("      " C_OPT "--{} <format>\n" C_LS "          {}\n\n", repl, hidden.find(repl, false, true).description());
                }
                return 0;
            } else if (boost::iequals(argument_01, "edit")) {
                // print spcific command
                fmt::print("Update entries structure recreating .vdf shortcuts file\n\n" C_TITLE "Usage:\n" C_LS " steam-shortcuts-editor edit [OPTIONS] <SHORTCUTS_PATH>\n\n" C_TITLE "Arguments:\n" C_LS
                           "  <SHORTCUTS_PATH>  Path to input (or/and eventually output) file \"shortcuts.vdf\"\n"
                           "\n" C_TITLE "Options:\n" C_LS);

                fmt::print("      " C_OPT "--json-path" C_LS " <JSON_PATH>  Path to json contains the entries. It will ignore --idx, --key, --val. If <SHORTCUTS_PATH> not exists, --json-path will be required.\n");
                fmt::print("      " C_OPT "--idx" C_LS " <IDX>              Index of the entry to operate on (requires --key and --val) if the entry does not exist idx will be ignored and a new one will be created to the end of the list\n");
                fmt::print("      " C_OPT "--key" C_LS " <KEY>              Single key to change on Shortcuts[idx] (requires --idx and --val)\n");
                fmt::print("      " C_OPT "--val" C_LS " <VAL>              New value for Shortcuts[idx].key=? (requires --idx and --key)\n");
                fmt::print("      " C_OPT "--out" C_LS " <VAL>              Output file destination for generated vdf. If <SHORTCUTS_PATH> is missing --out will be used as output.\n");
                fmt::print("      " C_OPT "--force" C_LS " <VAL>            Overwrite destination (--out) if exists.\n\n");
                return 0;
            } else {
                throw invalid_argument("Help command must be followed by a valid subcommand to inspect.");
            }
        }

        if (!boost::iequals(command, "list") && !boost::iequals(command, "edit")) {
            error$("ERROR", "Invalid command \"{}\"", command);
        }

        if (boost::iequals(command, "edit")) {
            if (argument_01.empty() && edit_options.out.empty())
                throw invalid_argument("Missing required <SHORTCUTS_PATH> or --out. Check the usage.");
            if (argument_01.empty() && edit_options.json_path.empty())
                throw invalid_argument("Missing required <SHORTCUTS_PATH> or --json-path. Check the usage.");
            if (edit_options.json_path.empty() && edit_options.key.empty())
                throw invalid_argument("Missing required --json-path or --id,--key,--val. Check the usage.");
        }

        if (argument_01.empty() && !boost::iequals(command, "edit"))
            throw invalid_argument("Missing input shortcuts file. Check the usage.");

        const fs::path vdf(argument_01);
        if (!fs::exists(vdf) && !boost::iequals(command, "edit")) {
            throw runtime_error(fmt::format("Shortcuts file not exists at: \"{}\"", vdf.string()));
        }
        Shortcuts scs;
        if (!argument_01.empty()) {
            auto vdf_fullpath = fs::canonical(vdf);

            scs.parse(vdf_fullpath);
        }

        if (boost::iequals(command, "list")) {
            const auto showkeys = vm.count("keys");
            const auto json = vm.count("json");
            show_list(format_switches, scs, separator, showkeys, json);
        } else if (boost::iequals(command, "edit")) {
            if (edit_options.out.empty())
                edit_options.out = argument_01;

            if (edit_options.out.empty())
                throw invalid_argument("Missing output (--out) shortcuts file. Check the usage.");

            const fs::path vdf_out(edit_options.out);
            if (fs::exists(vdf_out)) {
                if (!vm.count("force"))
                    throw runtime_error(fmt::format("Shortcuts file already exists at: \"{}\"", vdf_out.string()));               
            }

            if (!edit_options.json_path.empty()) {
                // yarn debug edit ../examples/shortcuts_1.vdf ../examples/cpp_output.vdf --json-path=../examples/cpp_output.json
#ifdef DEBUG
                log$("main", "Operate on json-path: {}", edit_options.json_path);
#endif
                const fs::path jfile(edit_options.json_path);
                if (!fs::exists(jfile)) {
                    throw runtime_error(fmt::format("Inpout json file not exists at: \"{}\"", jfile.string()));
                }

                scs.update_from_json_file(jfile);

            } else if (vm.count("idx")) {
#ifdef DEBUG
                log$("main", "Operate on keyvalue: [{}]{} = \"{}\"", edit_options.idx, edit_options.key, edit_options.val);
#endif
                // Validate key
                auto validate = Shortcuts::is_prop_valid(edit_options.key, edit_options.val);

                if (validate == PropsValidReturns::InvalidKey)
                    throw runtime_error(fmt::format("\"{}\" is not a supported key.", edit_options.key));

                if (validate == PropsValidReturns::InvalidValue)
                    throw runtime_error(fmt::format("\"{}\" is not a supported value for key \"{}\".", edit_options.val, edit_options.key));

                if (validate == PropsValidReturns::InvalidArrayValue)
                    throw runtime_error(fmt::format("\"{}\" must be a valid json array of strings for key \"{}\".", edit_options.val, edit_options.key));

                auto isnew = scs.get_or_create(edit_options.idx, [](bool isnew, Shortcut &sc) {
                    if (isnew) {
                        throw runtime_error(fmt::format("Schortcut with index={} is missing. Cannot create new one from one single key->value. Use --json-path instead.", edit_options.idx));
                    }
                    if (Shortcuts::prop_is_uint32(edit_options.key)) {
                        sc.props[edit_options.key] = (uint32_t)atoi(edit_options.val.c_str());
                    } else if (Shortcuts::prop_is_string(edit_options.key)) {
                        sc.props[edit_options.key] = edit_options.val;
                    } else if (Shortcuts::prop_is_stringarr(edit_options.key)) {
                        sc.props[edit_options.key] = Shortcuts::resja(edit_options.val);
                    }
                });
            } else {
                throw invalid_argument("Required --idx or --json-path <JSON_PATH>.");
            }

            log$("main", "Write to file: {}", edit_options.out);

            if (fs::exists(vdf_out)) {
                fs::remove(vdf_out);
            }

            scs.store_into(edit_options.out);
            // show_list(format_switches, scs, separator, false, true);
        } else {
            throw invalid_argument(fmt::format("Not supported command \"{}\"", command));
        }
    } catch (const po::multiple_occurrences &e) {
        error$("ERROR", "{}", "Cannot specify more than 2 positional arguments: <SHORTCUTS_PATH> <SUBCOMMAND>.");
        error$("main", "{}", "Try use --help.");
        error$("main", "{}", "Program aborted.");
    } catch (const exception &e) {
        error$("ERROR", "{}", /*typeid(e).name(), */ e.what());
        error$("main", "{}", "Program aborted.");
    }
}

void show_list(const unordered_map<string, string> &format_switches, Shortcuts &scs, const string &separator, bool showkeys, bool json) {
    const auto all_format = format_switches.at("all");
    auto out = fmt::memory_buffer();

    auto first_obj = true;

    if (json)
        format_to(std::back_inserter(out), "{}", "[");
    scs.foreach ([&first_obj, &json, &format_switches, &out, &all_format, &separator, &showkeys](const Shortcut &sc) {
                            auto first = true;
                if (json)
                {
                    if (!first_obj)
                        format_to(std::back_inserter(out), "{}", ",");
                    first_obj = false;
                    format_to(std::back_inserter(out), "{}", "{");
                    for (const auto &elem : sc.props)
                    {
                        if (elem.first == "all")
                            continue;

                        
                        auto val = sc.props.find(elem.first);
                        if (val != sc.props.end())
                        {
                            if (!first)
                                format_to(std::back_inserter(out), "{}", ",");
                            first = false;
                            format_to(std::back_inserter(out), "\"{}\":", elem.first);
                            if (auto st = get_if<string>(&val->second))
                            {
                                format_to(std::back_inserter(out), elem.first == "tags" ? "{}" : "\"{}\"", elem.first == "tags"  ? *st : std::regex_replace(*st, std::regex("\""), "\\\""));
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
                            //format_to(std::back_inserter(out), "{}", "<missing>");
                        }                        
                    }
                    format_to(std::back_inserter(out), "{}", "}");
                    
                }
                else
                {
                    // log$("main", "Cycle {}",sc.index);

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
                }
                    return true; });
    if (json)
        format_to(std::back_inserter(out), "{}\n", "]");
    fmt::print("{}", string(out.data(), out.size()));
}