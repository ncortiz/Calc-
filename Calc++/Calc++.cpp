#include <iostream>

#include "Calculator.h"

int main()
{
    const std::string program = "x = 5; while 1-x { print x; x = x - 1; };";

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
