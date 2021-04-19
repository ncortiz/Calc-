# Calc++

Simple turing complete interpreted programming language

Written in C++

Current features:

* ``while`` loops ``while x < 5 { if x % 2 == 0 { print x; }; x = x + 1; };`` 
* ``if`` statements:  ``if x % 4 == 3 { print x; };`` considers any value != 0 to be true and 0 to be false
* ``def`` statements: ``def name { ret 5; }; call name;``
* ``ret`` from function ``ret x;``
* ``print`` something ``print x;``
* ``out`` exit with the program returning value ``out -1;``
* ``call`` a function inside expression (argument is identifier of function)
* ``+ - * / % <= >= < > `` operators with precedence
* '()' parenthesis: can have any expression inside it (duh)
* ``not`` function: makes any value != 0 -> 0 and value == 0 -> 1
* ``sqrt`` function: cause why not
* ``time`` constant: returns time since program start in nanoseconds
* ``if def while `` all use bodies enclosed in ``{}``
* functions don't require parenthesis, but they're allowed as they are part of atom: ``( expr )``
* numeric literals: integers (currently long long type)
* type safety: only two types for now (num, func. ptr which can only be used to call functions)
* stack based architecture (only one stack for all variables and symbol table with reference to stack locations) also used for data from recursive descent
* compiler/interpreter runs from command line first argument is filename of program to run
* fast? (loops can run at less than 150 microseconds (1e-6) per iteration)

Todo:
* integrate parallelism (openmp? cuda -> for gpus?)
* input
* make stuff currently using ``{}`` to allow for single line statements as well
* encapsulate unsafe stuff better maybe actually use safer methods (check performance)
* memory management: allocation and freeing 
* arbitrary precision integers (maybe with fixed point, no decimals to avoid non-precision of floating points)
* other data types (strings as arrays of integers)
* arrays (on stack, with first num being size, we already have type safety)
* function ptrs as data types
* pointers and references (with rust type safety)
* implement on virtual machine by compiling code into byte code maybe adapter (onto AST first) so we can write an actual x86-64 compiler later and optimizer (some things will be difficult however as we cannot see data but already used stack structure still applies I guess)
* YES, create ASTs, that's better probably!
* more abstraction (compiler class as parent, implementations describe actual grammar, generic expr stuff and unsafe stuff on parent)




Example code (also as files in release directory, capp is set to be opened by Calc++.exe in order to act as executables python-style):

> Even nums.capp
````
i = 0;

while(i <= 30)
{
	if i % 2 == 0 {
		print i;
	};

	i = i + 1;
};

````

> Powers of two.capp
````

i = 0;
x = 2;

while(i <= 30)
{
	print x;
	x = x * 2;
	i = i + 1;
};

````

> Functions.capp
````

def add 
{
	x = 5 + 5 * 2;
	x = 0;
	
	if x == 0 { ret 55;}; 
	
	ret x + 1;
};

out (call add);

````
