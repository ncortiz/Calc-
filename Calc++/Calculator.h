#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
#include <algorithm>

#define IN_RANGE(a,z) (*cur_char >= a && *cur_char <= z)
#define EXPECT_CHAR(c) if(*cur_char != c) throw std::string("Expected '") + c + std::string("' but found character '") + *cur_char + "'"; next_char();

class Calculator
{
private:
    const char* program = 0;
    const char* cur_char = 0;
    size_t sz_program = 0;
    size_t sz_stack = 1000;
    size_t ln = 0, col = 0;
    std::vector<long long> stack;
    std::unordered_map<const char*, size_t> variable_map;

public:

    Calculator()
    {}

    void Clear ()
    {
        ln = col = 0;
        stack.clear ();
        variable_map.clear ();
    }

    long long Scan (const std::string& program, const size_t& sz_stack = 1000)
    {
        this->program = program.c_str();
        this->cur_char = this->program;
        this->sz_program = program.size ();
        this->sz_stack = sz_stack;

        try
        {
            scan_stmt ();
        }
        catch (const std::string & ex)
        {
            throw std::string ("[Runtime] Exception: ") + ex + " at " + get_location ();
        }
        catch (...)
        {
            throw std::string("[Runtime] Unknown exception") + " at " + get_location ();
        }

        try {
            return stack_pop ();
        }
        catch (...)
        {
            return 0;
        }
    }

    long long get_variable (const char* identifier)
    {
        auto search = variable_map.find (identifier);
        if (search == variable_map.end ())
            throw std::string ("Could not find variable '") + *cur_char + "'";

        return stack[search->second];
    }

    void set_variable (const char* identifier, long long value)
    {
        stack_push (value);
        variable_map[identifier] = stack.size ();
    }

private:

    std::string get_location ()
    {
        return std::string("[Ln:") + std::to_string(ln) + 
            ", Col : " + std::to_string(col) + 
            ", Ch : " + std::to_string(cur_char-program) + "]";
    }

    void remove_whitespace ()
    {
        while (*cur_char == ' ' || *cur_char == '\n' || *cur_char == '\t')
        {
            if (*cur_char == '\n')
            {
                ln++;
                col = 0;
            }
            else
                col += *cur_char == '\t' ? 5 : 1;

            next_char ();
        }
    }

    bool is_identifier ()
    {
        return IN_RANGE ('a', 'z') || IN_RANGE ('A', 'Z') || *cur_char == '_';
    }

    long long stack_pop ()
    {
        if (stack.empty ())
            throw std::string ("Stack is empty");

        auto v = stack.back ();
        stack.pop_back ();
        return v;
    }

    long long stack_push (long long value)
    {
        if (stack.size () >= sz_stack)
            throw std::string ("Stack overflow");
        stack.push_back (value);
    }

    std::string collect_identifier ()
    {
        std::string ident = "";
        while (is_identifier () || IN_RANGE (0, 9))
        {
            ident += *cur_char;
            next_char ();
        }
        return ident;
    }

    void next_char ()
    {
        if (program == 0)
            throw std::string ("Program is NULL");

        if ((cur_char - program) >= sz_program)
            throw std::string ("Unexpected EOF");

        cur_char++;
    }

    long long scan_literal ()
    {
#ifdef _DEBUG
        std::cout << "[literal]" << std::endl;
#endif
        if (!IN_RANGE ('0', '9'))
            throw std::string ("Expected numeric literal but found '") + *cur_char + "'";

        long long acc = 0;
        for (; IN_RANGE ('0', '9'); next_char())
        {
            acc = (acc * 10) + (*cur_char - '0');
            col++;
        }

        stack_push (acc);
    }

    void scan_atom ()
    {
        remove_whitespace ();

        if (is_identifier ())
        {
#ifdef _DEBUG
            std::cout << "[var ref]" << std::endl;
#endif

            auto ident = collect_identifier ();

            auto search = variable_map.find (ident.c_str());
            if (search == variable_map.end ())
                throw std::string ("Undeclared variable '") + ident + "'";

            stack_push (stack[search->second]);
        }
        else if (*cur_char == '(')
        {
            next_char ();

            scan_expr ();

            remove_whitespace ();

            EXPECT_CHAR (')');
        }
        else
            scan_literal ();
    }

    void scan_expr_generic (char op1, char op2, std::function<long long (void)> op1f, std::function<long long (void)> op2f, void(Calculator::*lower)())
    {
        (this->*lower) ();

        remove_whitespace ();

        char op = *cur_char;
        if (op != op1 && op != op2)
            return;
        next_char ();

#ifdef _DEBUG
        std::cout << "['" << op << "' expr]" << std::endl;
#endif

        remove_whitespace ();

        scan_expr_generic (op1, op2, op1f, op2f, lower);

        if (op == op1)
            stack_push (op1f ());
        else
            stack_push (op2f ());
    }

    void scan_expr_mul_div ()
    {
        scan_expr_generic ('*', '/', [&]() { return stack_pop () * stack_pop (); }, [&]() { return stack_pop () / stack_pop (); }, &Calculator::scan_atom);
    }

    void scan_expr_add_sub ()
    {
        scan_expr_generic ('+', '-', [&]() { return stack_pop () + stack_pop (); }, [&]() { return stack_pop () - stack_pop (); }, &Calculator::scan_expr_mul_div);
    }

    void scan_expr ()
    {
        return scan_expr_add_sub ();
    }

    void scan_stmt ()
    {
        remove_whitespace ();

        while (is_identifier ())
        {
            auto ident = collect_identifier ();

            auto ident_upper = ident;
            std::transform (ident_upper.begin (), ident_upper.end (), ident_upper.begin (), ::toupper);

            if (ident_upper == "OUT")
            {
#ifdef _DEBUG
                std::cout << "[out]" << std::endl;
#endif

                remove_whitespace ();
                scan_expr ();
                throw stack_pop ();
            }
            else if (ident_upper == "PRINT")
            {
#ifdef _DEBUG        
                std::cout << "[print]" << std::endl;
#endif
                remove_whitespace ();
                scan_expr ();
                std::cout << stack_pop ();
            }
            else
            {
#ifdef _DEBUG
                std::cout << "['var decl]" << std::endl;
#endif
                remove_whitespace ();
                EXPECT_CHAR ('=');
                remove_whitespace ();
                scan_expr ();
                variable_map[ident.c_str ()] = stack.size () - 1;
            }

            remove_whitespace ();
            EXPECT_CHAR (';');
            remove_whitespace ();
        }
    }
};