#include "./shortcuts.hpp"

#include <iostream>
#include <string>

using namespace std;

using namespace shortcuts;

void test()
{
    cout << "test1"
         << "\n";
}


void shortcuts::test3()
{
    cout << "test3"
         << "\n";
    test();
}

void Shortcuts::test() {
    cout << "Shortcuts::test"
         << "\n";
}

