#pragma once

#include <iostream>
#include <string>

namespace shortcuts
{
    class Shortcuts
    {
    public:
        Shortcuts()
        {
            std::cout << "Shortcuts: Constructed"
                      << "\n";
        }
        ~Shortcuts()
        {
            std::cout << "Shortcuts: Destroyed"
                      << "\n";
        }
        void test();
    };

    inline void test2()
    {
        std::cout << "test2"
                  << "\n";
    }

    void test3();

}
