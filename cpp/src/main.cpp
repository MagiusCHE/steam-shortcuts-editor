#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#define FMT_HEADER_ONLY
#include <boost/program_options.hpp>
#include "./shortcuts.hpp"
#include "./logging.hpp"
#include <chrono>
#include <thread>

using namespace std;

namespace fs = filesystem;

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    logging::update_debug_src_root_path(__FILE__);
    // log$("main", "{}", "Start");

    try
    {
        string ifile;

        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("list", "list all shortcuts")("input,i", po::value(&ifile), "pathname of shortcuts.vdf");
        // desc.add_options()("list", "list all shortcuts");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (ifile.empty())
            throw invalid_argument("Missing input shortcuts file.");

        const fs::path vdf(ifile);
        if (!fs::exists(vdf)){
            throw runtime_error(fmt::format("Shortcuts file not exists at: \"{}\"", vdf.string()));
        }

        auto vdf_fullpath = fs::canonical(vdf);

        //log$("main", "Analyze: {}", vdf_fullpath.string());

        auto hascommand = vm.count("list");

        if (vm.count("help") || !hascommand)
        {
            cout << desc << "\n";
            return 0;
        }

        // log$("main","Load file {}",vm.at())

        auto scs = shortcuts::Shortcuts(vdf_fullpath);

        // All Shortcuts will be created
        scs.parse();
    }
    catch (const exception &e)
    {
        error$("ERROR", "{}", e.what());
        error$("main", "{}", "Try use --help.");
        error$("main", "{}", "Program aborted.");
    }
}