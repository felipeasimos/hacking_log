# Compiler

sources:
http://ce.sharif.edu/courses/94-95/1/ce414-2/resources/root/Text%20Books/Compiler%20Design/Alfred%20V.%20Aho,%20Monica%20S.%20Lam,%20Ravi%20Sethi,%20Jeffrey%20D.%20Ullman-Compilers%20-%20Principles,%20Techniques,%20and%20Tools-Pearson_Addison%20Wesley%20(2006).pdf

## Overview

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

## Basic Concepts

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

## Summary Of Phases

From source code:

```
position = initial + rate * 60
```

1. __Lexical analysis (scanning)__ - Takes a stream of characters
making up the source code and groups them into meaningful sequences
called __lexemes__. For each __lexeme__, the lexical analyzer
produces as output a __token__ of the form:

<_token-name_,_attribute-value_>

Each token has a name and optionally a value.

```
<id,1> <=> <id,2> <+> <id,3> <*> <60>
```

2. __Syntax analysis__ - Generates syntax trees from
the tokens:

```   <=>
     /   \
    /     \
<id,1>    <+>
         /   \
	/     \
    <id,2>    <*>
             /   \
	    /     \
	<id,3>    <60>
```

3. __Semantic analysis__

	* Check semantic consistency
	* Gathers type information (saved in syntax tree)
	* Type checking: check if each operator has matching operands
	* Implicit conversions (inttofloat)

```   <=>
     /   \
    /     \
<id,1>    <+>
         /   \
	/     \
    <id,2>    <*>
             /   \
	    /     \
	<id,3>   inttofloat
		    |
		    |
		    60
```


4. __Intermediate code generation__ - Generate explicit low-level or
machine-like intermediate representation, which we can think of as
a program for an abstract machine (it is machine-independent). This
representation should:

	* Be easy to produce
	* Should be easy to translate into the target machine

An example of this is _three-address code_, which consists of a
sequence of assembly-like instructions with three operands per
instruction, each operand acting like a register:

```
t1 = inttofloat(60)
t2 = id3 * t1
t3 = id2 + t2
id1 = t3
```

5. __Code optimization__ - Get results at compile time. Eliminate
redundant code. For example, instead of calling `inttofloat` we
can just change `60` to `60.0`. In doing so, the line
`t1 = inttofloat(60)` is not necessary, since we can just
change `t1` in the other lines for `60.0`.

```
t2 = id3 * 60.0
t3 = id2 + t2
id1 = t3
```

6. __Code generation__ - Translates the independent code into
the target language. If the target is machine code, registers
or memory locations are selected for each variable used.

```
LDF R2,id3
MULF R2,R2, #60.0
LDF R1,id2
ADDF R1,R1,R2
STF id1,R1
```
