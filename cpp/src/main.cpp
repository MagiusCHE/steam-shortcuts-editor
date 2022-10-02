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

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    logging::update_debug_src_root_path(__FILE__);
    log$("main", "{}", "Start");
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cout << desc << "\n";
        return 0;
    }
    vector<string> msg{"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

    for (const string &word : msg)
    {
        cout << word << " ";
    }
    cout << endl;

    shortcuts::test2();
    shortcuts::test3();

    shortcuts::Shortcuts scs = shortcuts::Shortcuts();
    scs.test();

    // delete scs;
    std::this_thread::sleep_for(std::chrono::milliseconds(512));
    log$("main","{}", "End");
}