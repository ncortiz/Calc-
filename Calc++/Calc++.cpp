#include <iostream>

#include "Calculator.h"

int main()
{
    const std::string program = "5 + 3 * 2";

    Calculator calc;

    try
    {
        auto v = calc.Scan (program);
        std::cout << "Return value is " << v << std::endl;
        std::cin.get ();
    }
    catch (const std::string &ex)
    {
        std::cerr << ex << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cout << "Unknown exception" << std::endl;
        return -1;
    }


    return 0;
}
