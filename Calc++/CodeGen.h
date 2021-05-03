#pragma once

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>

static enum OpCode
{
	stop,
	push,
	pop,
	add,
	sub,
	mul,
	out,
	outnum,
	outstr,
	memwrite,
	memread,
	cmp,
	jmp,
	je,
	jne,
	jg,
	jng,
	jl,
	jnl,
	dup,
	dupt,
	swap,
	in,
	innum,
	dupn_offset,
	swapt,
	popn,
	outp,
	popn_offset
};

std::string to_string (OpCode opcode)
{
	switch (opcode)
	{
	case stop:
		return "stop";
	case push:
		return "push";
	case pop:
		return "pop";
	case add:
		return "add";
	case sub:
		return "sub";
	case mul:
		return "mul";
	case out:
		return "out";
	case outnum:
		return "outnum";
	case outstr:
		return "outstr";
	case memwrite:
		return "memwrite";
	case memread:
		return "memread"; 
	case cmp:
		return "cmp";
	case jmp:
		return "jmp";
	case je:
		return "je";
	case jne:
		return "jne";
	case jg:
		return "jg";
	case jng:
		return "jng";
	case jl:
		return "jl";
	case jnl:
		return "jnl";
	case dup:
		return "dup"; 
	case dupt:
		return "dupt";
	case swap:
		return "swap";
	case in:
		return "in";
	case innum:
		return "innum";
	case dupn_offset:
		return "dupn_offset";
	case swapt:
		return "swapt";
	case popn:
		return "popn";
	case outp:
		return "outp";
	case popn_offset:
		return "popn_offset";
	default:
		return std::to_string (opcode);
	}
}

uint64_t gen_instr (uint8_t op, uint32_t imm = 0)
{
	return (static_cast<uint64_t>(op) << 32) | static_cast<uint64_t>(imm);
}

void write (uint64_t* p, size_t n, const char* file)
{
	std::ofstream of (file, std::ios::binary | std::ios::out);

	if (!of)
		throw "Bad ofstream";

	of.write ((char*)&n, sizeof (n));
	of.write ((char*)p, sizeof (uint64_t) * n);

	of.close ();

	if (of.bad ())
		throw "Bad ofstream after write";
}

void dump (uint64_t* p, size_t n)
{
	for (int i = 0; i < n;)
		std::cout << (int)p[i++] << " ";
}

