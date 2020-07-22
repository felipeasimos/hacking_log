# Compiler

sources:
http://ce.sharif.edu/courses/94-95/1/ce414-2/resources/root/Text%20Books/Compiler%20Design/Alfred%20V.%20Aho,%20Monica%20S.%20Lam,%20Ravi%20Sethi,%20Jeffrey%20D.%20Ullman-Compilers%20-%20Principles,%20Techniques,%20and%20Tools-Pearson_Addison%20Wesley%20(2006).pdf

## Section 1

### Section 1.1

* __Compiler__ - Program that reads a program in _source_ language and
translate it to an equivalent program in a _target_ language, reporting
any errors it may find in the source program. The target program
can then be executed to process input and produce outputs.

```
          .----------.
source -> | compiler | -> target
	  `----------´
          .--------.
inputs -> | target | -> outputs
	  `--------´
```

* __Interpreter__ - Instead of producing a target program, it directly
execute the operations specified in the source program
(statement by statement) on inputs supplied by the user. Cut out the
middleman! Much slower than a compiler, but can give better error
diagnostics.

```
          .-------------.
source -> |             |
          | interpreter | -> outputs
inputs -> |             |
          `-------------´
```

Pratical Example.: Java combines compilation and interpretation,
first translating the source in _bytecode_ which is then
intepreted by a virtual machine.

To achieve faster processing of inputs to outputs, some Java compilers,
called _just-in-time_ compilers, translate the _bytecodes_ into
machine language immediately before they run the intermediate
program to process input.

```
	  source
	    |
 	    v
     .------------.
     | translator |
     `------------´
	    |
	    v               .-----------------.
   intermediate program ->  |                 |
                            | virtual machine | -> output
	         inputs ->  |                 |
		            `-----------------´
```

Several other programs may be used to generate a target, like
a _preprocessor_, a _linker_, a _assembler_, a _loader_, etc.

```
		  .--------------.                        .----------.
	source -> | preprocessor | -> modified source ->  | compiler | -> target assembly
		  `--------------´                        `----------´           |
                                                                                 v
			.-----------------.			           .-----------.
 target machine code <- | linker / loader | <- relocatable machine code <- | assembler |
			`-----------------´     			   `-----------´
                            ^        ^
			    |        |
		      library files  |
		                     |
			 relocatable object files
```

### Section 1.2

Compiler is divided in two parts:

* __analysis__ (_front end_ of the compiler) - breaks up source
in _constituent_ pieces and impose grammatical structure to
them. It then uses this structure to create an intermediate
representation of the source program.

	* It will provide informative messages if it notices that the source
	is semantically or syntactically ill formed unsound.

	* It also stores information about the source in a data structure
	called __symbol table__, which is passed along with the intermediate
	representation to the synthesis part.

* __synthesis__ (_back end_ of the compiler) - constructs the
desired target from the intermediate representation and
the __symbol table__.

Compilation operates in a sequence of __phases__, each transforming one
representation of the program into another. In practice, several phases
may be grouped together and the intermediate representations between the
grouped __phases__ need not to be constructed explicitly.

The __symbol table__ will be used by all __phases__.

![Compiler phases](https://www.tutorialspoint.com/compiler_design/images/compiler_phases.jpg)

Some compilers have machine-independent optimization phases between 
the front and the back end. It is optional so one or two optimization
phases may be missing.

#### Lexical Analysis

_Lexical Analysis_ or _scanning_ is the first phase of the
compiler. It takes in a stream of chracters making up the
source program and groups them into meaningful sequences
called __lexemes__. For each __lexeme__, the lexical
analyzer produces as output a __token__ of the form:

```
<token_name, attribute_value>
```

* __`token_name`__ - abstract symbol used during syntax
analysis.

* __`attribute_value`__ - points to an entry to the symbol table
for this token.

the token is passed to the subsequent phase, syntax analysis.

#### Summary

1. Lexical analysis - Generate tokens

2. Syntax analysis - Generates token trees

3. Semantic analysis 

	* Check semantic consistency
	* Gathers type information (saved in syntax tree)
	* Type checking: check if each operator has matching operands
	* Implicit conversions

4. Intermediate code generation - 
