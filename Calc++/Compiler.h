#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <string>
#include <algorithm>
#include <chrono>
#include "CodeGen.h"

#define IN_RANGE(a,z) (*cur_char >= a && *cur_char <= z)
#define EXPECT_CHAR(c) if(*cur_char != c) throw std::string("Expected '") + c + std::string("' but found character '") + *cur_char + "'"; next_char();

class Compiler
{
private:
	const char* program = 0;
	const char* cur_char = 0;
	size_t sz_program = 0;
	size_t sz_stack = 1000;
	size_t ln = 0, col = 0;
	size_t stack_ptr = 0;
	//std::vector<long long> stack;
	bool return_from_function = false;

	std::vector<uint64_t> compiled_program;

	enum type
	{
		num,
		func_ptr
	};

	enum scope
	{
		global,
		func
	};

	scope current_scope = scope::global;
	std::map<std::string, std::pair<size_t, type>> variable_map;
	std::chrono::steady_clock::time_point begin;

public:

	Compiler
	()
	{}

	void Clear ()
	{
		current_scope = scope::global;
		ln = col = 0;
		stack_ptr = 0;
		variable_map.clear ();
	}

	void Scan (const std::string& program, const size_t& sz_stack = 1000)
	{
		this->program = program.c_str ();
		this->cur_char = this->program;
		this->sz_program = program.size ();
		this->sz_stack = sz_stack;
		stack_ptr = 0;

		try
		{
			begin = std::chrono::steady_clock::now ();
			scan_stmt ();
			clean_scope ();
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
			throw std::string ("[Runtime] Unknown exception") + " at " + get_location ();
		}
	}

private:

	std::string get_location ()
	{
		return std::string ("[Ln:") + std::to_string (ln) +
			", Col : " + std::to_string (col) +
			", Ch : " + std::to_string (cur_char - program) + "]";
	}

	void remove_whitespace ()
	{
		while (*cur_char == ' ' || *cur_char == '\n' || *cur_char == '\t')
			next_char ();
	}

	bool is_identifier ()
	{
		return IN_RANGE ('a', 'z') || IN_RANGE ('A', 'Z') || *cur_char == '_';
	}

	void stack_pop ()
	{
		if (stack_ptr == 0)
			throw std::string ("Stack is empty");

		stack_ptr--;
	}

	void emit_instruction (OpCode opcode, unsigned long long imm=0)
	{
		if(opcode == OpCode::push)
			std::cout << to_string (opcode) << " " << imm << std::endl;
		else
			std::cout << to_string(opcode) << std::endl;

		compiled_program.push_back (gen_instr(opcode, imm));
	}

	void stack_push ()
	{
		if (stack_ptr >= sz_stack)
			throw std::string ("Stack overflow");
		stack_ptr++;
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

		if (*cur_char == '\n')
		{
			ln++;
			col = 0;
		}
		else
			col += *cur_char == '\t' ? 5 : 1;

		cur_char++;
	}

	long long scan_literal ()
	{
#ifdef _DEBUG
		std::cout << "[literal]" << std::endl;
#endif
		if (!IN_RANGE ('0', '9'))
			throw std::string ("Expected numeric literal but found '") + *cur_char + "'";

		unsigned long long acc = 0;
		for (; IN_RANGE ('0', '9'); next_char ())
		{
			acc = (acc * 10) + (*cur_char - '0');
		}

		emit_instruction (OpCode::push, acc);
		stack_push ();
	}

	void scan_atom ()
	{
		remove_whitespace ();

		if (is_identifier ())
		{
			auto ident = collect_identifier ();

			auto ident_upper = ident;
			std::transform (ident_upper.begin (), ident_upper.end (), ident_upper.begin (), ::toupper);

			if (ident_upper == "SQRT")
			{
#ifdef _DEBUG
				std::cout << "[sqrt]" << std::endl;
#endif
				throw std::string ("Unimplemented");

				scan_expr ();
				stack_pop ();
				//emit_instruction (OpCode::sqrt);
				//
				stack_push ();
			}
			else if (ident_upper == "NOT")
			{
#ifdef _DEBUG
				std::cout << "[not]" << std::endl;
#endif
				throw std::string ("Unimplemented");
				scan_expr ();
				stack_pop ();
				//emit_instruction (OpCode::not);
				//
				stack_push ();
			}
			else if (ident_upper == "TIME")
			{
#ifdef _DEBUG
				std::cout << "[time]" << std::endl;
#endif
				throw std::string ("Unimplemented");

				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now ();
				//stack_push (std::chrono::duration_cast<std::chrono::nanoseconds>(now - begin).count ());

				//emit_instruction (OpCode::time);
				stack_push ();
			}
			else if (ident_upper == "CALL")
			{
#ifdef _DEBUG        
				std::cout << "[call]" << std::endl;
#endif
				remove_whitespace ();

				if (!is_identifier ())
					throw std::string ("Expected identifier");

				auto ident = collect_identifier ();

				std::map<std::string, std::pair<size_t, type>>::iterator search = variable_map.find (ident);
				if (search == variable_map.end ())
					throw std::string ("Undeclared function '") + ident + "'";

				if (search->second.second != type::func_ptr)
					throw std::string ("Invalid call of variable of type num");

				auto ptr = variable_map[ident].first;

				if (ptr < 0 || ptr >= compiled_program.size())
					throw std::string ("Invalid function pointer");

				emit_instruction (OpCode::push, ptr);
				emit_instruction (OpCode::jmp);
			}
			else
			{
#ifdef _DEBUG
				std::cout << "[var ref '" << ident << "']" << std::endl;
#endif

				std::map<std::string, std::pair<size_t, type>>::iterator search = variable_map.find (ident);
				if (search == variable_map.end ())
					throw std::string ("Undeclared variable '") + ident + "'";

				if (search->second.second != type::num)
					throw std::string ("Invalid use of variable of type function ptr");

				emit_instruction (OpCode::push, search->second.first);
				emit_instruction (OpCode::dup);
				stack_push ();
			}
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

	void clean_scope ()
	{
		while (stack_ptr > 0)
		{
			stack_pop ();
			emit_instruction (OpCode::pop);
		}
	}

#pragma region Generic
	void scan_expr_generic (char op1, char op2, char op3, char op4, std::function<void (void)> op1f, std::function<void (void)> op2f, std::function<void (void)> op3f, std::function<void (void)> op4f, void(Compiler::* lower)())
	{
		(this->*lower) ();

		remove_whitespace ();

		char op = *cur_char;
		if (op != op1 && op != op2 && op != op3 && op != op4)
			return;
		next_char ();

#ifdef _DEBUG
		std::cout << "['" << op << "' expr]" << std::endl;
#endif

		remove_whitespace ();

		scan_expr_generic (op1, op2, op3, op4, op1f, op2f, op3f, op4f, lower);

		if (op == op1)
			op1f ();
		else if (op == op2)
			op2f ();
		else if (op == op3)
			op3f ();
		else if (op == op4)
			op4f ();
	}

	void scan_expr_generic (char op1, char op2, char op3, std::function<void (void)> op1f, std::function<void (void)> op2f, std::function<void (void)> op3f, void(Compiler::* lower)())
	{
		(this->*lower) ();

		remove_whitespace ();

		char op = *cur_char;
		if (op != op1 && op != op2 && op != op3)
			return;
		next_char ();

#ifdef _DEBUG
		std::cout << "['" << op << "' expr]" << std::endl;
#endif

		remove_whitespace ();

		scan_expr_generic (op1, op2, op3, op1f, op2f, op3f, lower);

		if (op == op1)
			op1f ();
		else if (op == op2)
			op2f ();
		else if (op == op3)
			op3f ();
	}

	void scan_expr_generic (char op1, char op2, std::function<void (void)> op1f, std::function<void (void)> op2f, void(Compiler::* lower)())
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
			op1f ();
		else
			op2f ();
	}

	void scan_expr_generic (std::string op1, std::string op2, char op3, char op4, std::function<void (void)> op1f, std::function<void (void)> op2f, std::function<void (void)> op3f, std::function<void (void)> op4f, void(Compiler::* lower)())
	{
		(this->*lower) ();

		remove_whitespace ();

		std::string op = "";

		char first_char = *cur_char;
		if (*cur_char != op1[0] && *cur_char != op2[0] && *cur_char != op3 && *cur_char != op4)
			return;
		op += *cur_char;
		next_char ();
		if (*cur_char != op1[1] && *cur_char != op2[1] && (first_char == op3 || first_char == op4))
		{
			next_char ();

#ifdef _DEBUG
			std::cout << "['" << first_char << "' expr]" << std::endl;
#endif

			remove_whitespace ();

			scan_expr_generic (op1, op2, op3, op4, op1f, op2f, op3f, op4f, lower);

			if (first_char == op3)
				op3f ();
			else
				op4f ();

			return;
		}
		else if (*cur_char != op1[1] && *cur_char != op2[1])
			throw std::string ("Expected '") + op1[1] + "' or '" + op2[1] + "' but found character '" + *cur_char + "'";

		op += *cur_char;
		next_char ();

#ifdef _DEBUG
		std::cout << "['" << op << "' expr]" << std::endl;
#endif

		remove_whitespace ();

		scan_expr_generic (op1, op2, op3, op4, op1f, op2f, op3f, op4f, lower);

		if (op == op1)
			op1f ();
		else
			op2f ();

	}

	void scan_expr_generic (std::string op1, std::string op2, std::string op3, std::string op4, std::function<void (void)> op1f, std::function<void (void)> op2f, std::function<void (void)> op3f, std::function<void (void)> op4f, void(Compiler::* lower)())
	{
		(this->*lower) ();

		remove_whitespace ();

		std::string op = "";
		if (*cur_char != op1[0] && *cur_char != op2[0] && *cur_char != op3[0] && *cur_char != op4[0])
			return;
		op += *cur_char;
		next_char ();
		if (*cur_char != op1[1] && *cur_char != op2[1] && *cur_char != op3[1] && *cur_char != op4[1])
			throw std::string ("Expected '") + op1[1] + "', '" + op2[1] + "' or '" + op3[1] + "' or '" + op4[1] + "' but found character '" + *cur_char + "'";

		op += *cur_char;
		next_char ();

#ifdef _DEBUG
		std::cout << "['" << op << "' expr]" << std::endl;
#endif

		remove_whitespace ();

		scan_expr_generic (op1, op2, op3, op4, op1f, op2f, op3f, op4f, lower);

		if (op == op1)
			op1f ();
		else if (op == op2)
			op2f ();
		else if (op == op3)
			op3f ();
		else
			op4f ();
	}

	void scan_expr_generic (std::string op1, std::string op2, std::string op3, std::function<void (void)> op1f, std::function<void (void)> op2f, std::function<void (void)> op3f, void(Compiler::* lower)())
	{
		(this->*lower) ();

		remove_whitespace ();

		std::string op = "";
		if (*cur_char != op1[0] && *cur_char != op2[0] && *cur_char != op3[0])
			return;
		op += *cur_char;
		next_char ();
		if (*cur_char != op1[1] && *cur_char != op2[1] && *cur_char != op3[1])
			throw std::string ("Expected '") + op1[1] + "', '" + op2[1] + "' or '" + op3[1] + "' but found character '" + *cur_char + "'";

		op += *cur_char;
		next_char ();

#ifdef _DEBUG
		std::cout << "['" << op << "' expr]" << std::endl;
#endif

		remove_whitespace ();

		scan_expr_generic (op1, op2, op3, op1f, op2f, op3f, lower);

		if (op == op1)
			op1f ();
		else if (op == op2)
			op2f ();
		else
			op3f ();
	}

	void scan_expr_generic (std::string op1, std::string op2, std::function<void (void)> op1f, std::function<void (void)> op2f, void(Compiler::* lower)())
	{
		(this->*lower) ();

		remove_whitespace ();

		std::string op = "";
		if (*cur_char != op1[0] && *cur_char != op2[0])
			return;
		op += *cur_char;
		next_char ();
		if (*cur_char != op1[1] && *cur_char != op2[1])
			throw std::string ("Expected '") + op1[1] + "' or '" + op[1] + "' but found character '" + *cur_char + "'";

		op += *cur_char;
		next_char ();

#ifdef _DEBUG
		std::cout << "['" << op << "' expr]" << std::endl;
#endif

		remove_whitespace ();

		scan_expr_generic (op1, op2, op1f, op2f, lower);

		if (op == op1)
			op1f ();
		else
			op2f ();
	}

#pragma endregion Generic

	void scan_expr_mul_div ()
	{
		scan_expr_generic ('*', '/', '%',
			[=]() { 
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push (); 
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			}
			, &Compiler::scan_atom);
	}

	void scan_expr_add_sub ()
	{
		scan_expr_generic ('+', '-',
			[=]() { 
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::add);
				stack_push ();  
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::sub);
				stack_push ();
			}, &Compiler::scan_expr_mul_div);
	}

	void scan_expr_relational ()
	{
		scan_expr_generic ("<=", ">=", '<', '>',
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			}, &Compiler::scan_expr_add_sub);
	}

	void scan_expr_equality ()
	{
		scan_expr_generic ("==", "!=",
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			},
			[=]() {
				stack_pop ();
				stack_pop ();
				emit_instruction (OpCode::mul);
				stack_push ();
			},
			&Compiler::scan_expr_relational);
	}

	void scan_expr ()
	{
		return scan_expr_equality ();
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
				stack_pop ();
				emit_instruction (OpCode::out);
			}
			else if (ident_upper == "RET")
			{
#ifdef _DEBUG
				std::cout << "[out]" << std::endl;
#endif

				if (current_scope != scope::func)
					throw std::string ("Must be in function scope to return");

				remove_whitespace ();
				scan_expr ();

				stack_pop ();
				//emit_instruction (OpCode::ret);

				return_from_function = true;
				return;
			}
			else if (ident_upper == "PRINT")
			{
#ifdef _DEBUG        
				std::cout << "[print]" << std::endl;
#endif
				remove_whitespace ();
				scan_expr ();
				stack_pop ();
				emit_instruction (OpCode::outnum);
			}
			else if (ident_upper == "IF")
			{
#ifdef _DEBUG        
				std::cout << "[if]" << std::endl;
#endif
				remove_whitespace ();

				scan_expr ();
				stack_pop (); //cond

				emit_instruction (OpCode::push, 0);
				stack_push ();
				emit_instruction (OpCode::cmp);
				stack_pop ();

				auto prev_scope = stack_ptr;

				emit_instruction (OpCode::push, compiled_program.size ()); //TODO JUMP past if false OR 
				//WE NEED TO figure out where is past in order to jump past.!!!!
				stack_push ();
				emit_instruction (OpCode::je);
				stack_pop ();

				remove_whitespace ();
				EXPECT_CHAR ('{');

				scan_stmt ();

				remove_whitespace ();
				EXPECT_CHAR ('}');

				while (compiled_program.size () > stack_ptr)
				{
					stack_pop ();
					emit_instruction (OpCode::pop);
				}

			}
			else if (ident_upper == "WHILE")
			{
#ifdef _DEBUG        
				std::cout << "[while]" << std::endl;
#endif
				auto start = cur_char;

				remove_whitespace ();
				scan_expr ();
				stack_pop (); //cond

				remove_whitespace ();
				EXPECT_CHAR ('{');

				scan_stmt ();

				remove_whitespace ();
				EXPECT_CHAR ('}');
			}
			else if (ident_upper == "DEF")
			{
#ifdef _DEBUG        
				std::cout << "[def]" << std::endl;
#endif
				remove_whitespace ();

				if (!is_identifier ())
					throw std::string ("Expected identifier");

				auto ident = collect_identifier ();
				variable_map[ident] = std::make_pair (compiled_program.size()-1, type::func_ptr);

				remove_whitespace ();
				EXPECT_CHAR ('{');

				scan_stmt ();

				remove_whitespace ();
				EXPECT_CHAR ('}');
			}
			else
			{
				remove_whitespace ();
				EXPECT_CHAR ('=');
				remove_whitespace ();
				scan_expr ();

				if (variable_map[ident].second != type::num)
					throw std::string ("Attempted to reassign value with expr of invalid type");

				stack_pop ();
				variable_map[ident] = std::make_pair (stack_ptr+0, type::num);

				stack_push ();
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