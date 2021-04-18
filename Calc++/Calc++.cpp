#include <iostream>
#include <bitset>

#include "Calculator.h"

class BigNum
{
public:
    typedef std::vector<int> lnum;
    const int base = 1e9;
    lnum value;

    BigNum (const std::string &s)
    {
        for (int i = (int)s.length (); i > 0; i -= 9)
            if (i < 9)
                value.push_back (atoi (s.substr (0, i).c_str ()));
            else
                value.push_back (atoi (s.substr (i - 9, 9).c_str ()));
    }

    BigNum (lnum v)
        : value(v)
    {
    }

    BigNum &operator =(const BigNum& other)
    {
        value = other.value;
    }

    BigNum operator +(const BigNum& other)
    {
        auto a = value;
        auto b = other.value;

        int carry = 0;
        for (size_t i = 0; i < std::max (a.size (), b.size ()) || carry; ++i) {
            if (i == a.size ())
                a.push_back (0);
            a[i] += carry + (i < b.size () ? b[i] : 0);
            carry = a[i] >= base;
            if (carry)  a[i] -= base;
        }

        return BigNum(a);
    }

    BigNum operator -(const BigNum& other)
    {
        auto a = value;
        auto b = other.value;

        int carry = 0;
        for (size_t i = 0; i < b.size () || carry; ++i) {
            a[i] -= carry + (i < b.size () ? b[i] : 0);
            carry = a[i] < 0;
            if (carry)  a[i] += base;
        }
        while (a.size () > 1 && a.back () == 0)
            a.pop_back ();

        return BigNum (a);
    }

    BigNum operator *(const BigNum& other)
    {
        auto a = value;
        auto b = other.value;

        lnum c (a.size () + b.size ());
        for (size_t i = 0; i < a.size (); ++i)
            for (int j = 0, carry = 0; j < (int)b.size () || carry; ++j) {
                long long cur = c[i + j] + a[i] * 1ll * (j < (int)b.size () ? b[j] : 0) + carry;
                c[i + j] = int (cur % base);
                carry = int (cur / base);
            }
        while (c.size () > 1 && c.back () == 0)
            c.pop_back ();

        return BigNum (c);
    }

    BigNum operator /(const long long& b)
    {
        auto a = value;

        int carry = 0;
        for (int i = (int)a.size () - 1; i >= 0; --i) {
            long long cur = a[i] + carry * 1ll * base;
            a[i] = int (cur / b);
            carry = int (cur % b);
        }
        while (a.size () > 1 && a.back () == 0)
            a.pop_back ();

        return BigNum (a);
    }

    BigNum operator %(const long long& b)
    {
        auto a = value;

        int carry = 0;
        for (int i = (int)a.size () - 1; i >= 0; --i) {
            long long cur = a[i] + carry * 1ll * base;
            a[i] = int (cur / b);
            carry = int (cur % b);
        }
        while (a.size () > 1 && a.back () == 0)
            a.pop_back ();

        return BigNum (std::to_string(carry));
    }

    void print ()
    {
        printf ("%d", value.empty () ? 0 : value.back ());
        for (int i = (int)value.size () - 2; i >= 0; --i)
            printf ("%09d", value[i]);
    }

    friend std::ostream& operator<<(std::ostream& os, const BigNum& bn);

};

std::ostream& operator<<(std::ostream& os, const BigNum& bn)
{
    auto value = bn.value;
    
    if (value.empty())
        os << "0";

    while (!value.empty ())
    {
        os << value.back ();
        value.pop_back ();
    }

    return os;
}

int main()
{
    //
    const std::string program = "x = 25; while (x + 1) >= 0 { if (x % 2) == 1 { print x; }; x = x - 1; }; out time / 1000;";



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

    /*BigNum num ("2");

    std::cout << std::endl << 1 << std::endl;

    for (int i = 1; i < 62; i++)
    {
        std::cout << std::endl << num << std::endl;
        num = num * BigNum("2");
    }*/

    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();

    return 0;
}
