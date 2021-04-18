#pragma once

#include <iostream>
#include <vector>
#include <map>
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
    std::map<std::string, size_t> variable_map;

public:

    Calculator()
    {}

    void Clear ()
    {
        ln = col = 0;
        stack.clear ();
        variable_map.clear ();
    }

    void Scan (const std::string& program, const size_t& sz_stack = 1000)
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
        catch (const long long& ret_v)
        {
            throw;
        }
        catch (...)
        {
            throw std::string("[Runtime] Unknown exception") + " at " + get_location ();
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
            auto ident = collect_identifier ();

#ifdef _DEBUG
            std::cout << "[var ref '" << ident << "']" << std::endl;
#endif

            std::map<std::string, size_t>::iterator search = variable_map.find (ident);
            if (search == variable_map.end ())
                throw std::string ("Undeclared variable '") + ident + "'" ;

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

    void scan_expr_generic(char op1, char op2, char op3, std::function<long long(void)> op1f, std::function<long long(void)> op2f, std::function<long long(void)> op3f, void(Calculator::* lower)())
    {
        (this->*lower) ();

        remove_whitespace();

        char op = *cur_char;
        if (op != op1 && op != op2 && op != op3)
            return;
        next_char();

#ifdef _DEBUG
        std::cout << "['" << op << "' expr]" << std::endl;
#endif

        remove_whitespace();

        scan_expr_generic(op1, op2, op3, op1f, op2f, op3f, lower);

        if (op == op1)
            stack_push(op1f());
        else if(op == op2)
            stack_push(op2f());
        else if (op == op3)
            stack_push(op3f());
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
        scan_expr_generic ('*', '/', '%', 
            [&]() { return stack_pop () * stack_pop (); }, 
            [&]() {
                auto b = stack_pop();
                auto a = stack_pop();
                return a / b;
            },
            [&]() {
                auto b = stack_pop();
                auto a = stack_pop();
                return a % b;
            }
        , &Calculator::scan_atom);
    }

    void scan_expr_add_sub ()
    {
        scan_expr_generic ('+', '-', 
            [&]() { return stack_pop () + stack_pop (); }, 
            [&]() {
                auto b = stack_pop();
                auto a = stack_pop();
                return a - b; 
            }, &Calculator::scan_expr_mul_div);
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
                std::cout << stack_pop () << std::endl;
            }
            else if (ident_upper == "IF")
            {
#ifdef _DEBUG        
                std::cout << "[if]" << std::endl;
#endif
                remove_whitespace();

                scan_expr();
                auto cond = stack_pop(); 

                remove_whitespace();
                EXPECT_CHAR('{');

                if (cond != 0)
                    scan_stmt();
              
                remove_whitespace();
                EXPECT_CHAR('}');
            }
            else if (ident_upper == "WHILE")
            {
#ifdef _DEBUG        
                std::cout << "[while]" << std::endl;
#endif
                auto start = cur_char;

                remove_whitespace();
                scan_expr();
                auto cond = stack_pop();
                const char* end = cur_char;

                while (cond > 0)
                {
                    remove_whitespace();
                    EXPECT_CHAR('{');

                    scan_stmt();//TODO PROBLEM WE NEED TO CONSUME ALL AT LEAST ONCE
                    //THE ONLY THING THAT WOULD WORK WITHOUT THAT IS DO_WHILE's and GOTO with labels

                    remove_whitespace();
                    EXPECT_CHAR('}');

                    end = cur_char;
                    cur_char = start;
                    remove_whitespace();
                    scan_expr();
                    cond = stack_pop();
                }

                cur_char = end;
                
            }
            else
            {
                remove_whitespace ();
                EXPECT_CHAR ('=');
                remove_whitespace ();
                scan_expr ();
                variable_map[ident] = stack.size () - 1;
#ifdef _DEBUG
                std::cout << "[var decl '" << ident << "']" << std::endl;
#endif
            }

            remove_whitespace ();
            EXPECT_CHAR (';');
            remove_whitespace ();
        }
    }
};