#include <iostream>
#include <bitset>
#include <fstream>
#include <sstream>

#include "Calculator.h"

int main(int argc, char **argv)
{
    //const std::string program = "x = 25; while (x + 1) >= 0 { if (x % 2) == 1 { print x; }; x = x - 1; }; out time / 1000;";

    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " FILENAME" << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get ();
        return -1;
    }

    std::string filename = argv[1];
    std::string program = "";
    std::ifstream infile (filename);

    if (infile.bad ())
    {
        std::cerr << "Error: could not read file " << filename << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get ();
        return -1;
    }

    std::string line;
    while (std::getline (infile, line))
        program += line;

    Calculator calc;

    try
    {
        calc.Scan (program);
    }
    catch (const std::string &ex)
    {
        std::cerr << ex << std::endl;
        return -1;
    }
    catch (const long long& ret_v)
    {
        std::cout << "Return value is " << ret_v << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown exception" << std::endl;
        return -1;
    }

    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();

    return 0;
}
