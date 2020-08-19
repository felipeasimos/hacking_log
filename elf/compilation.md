# Compilation

sources:
https://www.calleerlandsson.com/the-four-stages-of-compiling-a-c-program/ \
https://codeforwin.org/2017/08/c-compilation-process.html \
https://www.cs-fundamentals.com/c-programming/how-to-compile-c-program-using-gcc \
https://www.geeksforgeeks.org/compiling-a-c-program-behind-the-scenes/ \
https://stackoverflow.com/questions/6264249/how-does-the-compilation-linking-process-work \
https://www.toptal.com/c-plus-plus/c-plus-plus-understanding-compilation \
https://www.tecmint.com/understanding-shared-libraries-in-linux/ \
http://osr507doc.sco.com/en/tools/ShLib_WhatIs.html \
https://cs-fundamentals.com/tech-interview/c/difference-between-static-and-dynamic-linking \
https://askubuntu.com/questions/690631/executables-vs-shared-objects \

How does your `.c` files can produce an ELF?

How can we generate different types of ELF files, and why are each of
these types important?

## The Compilation Stages

There are 4 stages of compilation. Each one is independent from the other,
and because of it `gcc` can produce the resulting file that each one
generates.

### Preprocessing

Preprocessing, or Pre-processing, is the first stage of compilation. It is
responsible for:

1. Removing comments from the source code, so the next stages never see
those.

2. Conditional compilation. The `#if` and `#else` macros can be used
to tell the preprocessor to ignore and essentially delete (so the
other stages don't use it too) pieces of code.

3. Macro expansion. If you have a line that reads `#define PI 3.14`, every
occurence of the string `PI` in your code will be switched to `3.14`. This
affects every line of code starting with a `#`. Another example is the
`#include` macro, in it the preprocessor switch the `#include` statement
by the source code of the file it is refering to.

	* Add special markers so the next stages know where each line of code
	came from and produce useful error messages. Error messages can also be
	generated in this stage by the use of `#if` and `#error` macros as well
	as invalid syntax.

All these changes are saved in a temporary `.i` file, which normally is
deleted after compilation. Like the others stages, the Preprocessing is
independent: it takes in a `.c` file and produce a `.i` one, without
caring what will happen to it. Because of it, what `gcc` does internally
is just call a program called `cpp` (acronym for C Preprocessor) that
handles the preprocessing phase entirely on its own.

We can generate an `.i` file with `gcc`:

`gcc -E <.c file> > <.i filename>`

Or directly with `cpp`:

`cpp <.c file> > <.i filename>`

One important aspect of this stage is that it doesn't read C code at any
point. You can easily test this: write a simple "hello world"
program. If you type `#inclde` instead of `#include`, `gcc -E` and `cpp`
will point out this is invalid syntax. However, if you type `man`
instead of `main` or `in` instead of `int`, no error messages will be
displayed.

### Compilation

Yes, the second stage of compilation is also called compilation. The
temporary preprocessed `.i` file is the input given to this stage
and the following actions are performed on it:

1. Check C code for syntax errors.

2. Translate C code to assembly functions specific to  the target
processor architecture.

This stage generate an temporary `.s` file containing the assembly code,
which can be saved with the command `gcc -S <.c file>`.

### Assembler

This stage turns the previous stage assembly code into machine code. Some
compilers have an integrated assembler in the compilation stage, which let
them generate machine code directly from the preprocessed C code.

Just like the in Preprocessing stage, the assembler has its own program
that is internally called by `gcc`: `as`, which generate an ELF file
ending with a `.o` (`.obj` in Windows), called an **object** file.
It can also be generated with `gcc -c`.

`as <.s file> -o <.o filename>` output file is the first ELF file
produced in the compilation process. It is an relocatable ELF file,
which means symbols like `printf` still need to be linked to their
definitions.

We can see the object file symbol table with the `nm` command. Undefined
symbols (which aren't linked with their definitions yet) appear with an
`U` before them.

Obs .: if you print "hello world" using `printf` with an newline in the end
the compiler may optimize your code by switching `printf` to `puts`. You
can verify it using the `nm` command with the `.o` file or just reading the
`.s` file normally and looking for `call` assembly commands.

### Linking

In this final stage, the undefined symbols are linked to their definitions
and an ELF file is generated. This file can be of the following types:

* __Executable__ - Object ready for execution, with position-dependent
code. Can't be linked to other executables or shared objects.

* __Shared Object__ - Object file with position-independent code that can
be linked to other shared objects or executables. Just like executables,
shared objects can be executaded, given that they have a `main` function.

If you compile a simple "hello world" `.c` file with
`gcc -no-pie <.c file>`, an executable will be generated. The `-no-pie`
flag avoid creating an position-independent object file. However, using
`gcc -pie -fPIC <.c file>` generates an shared object that can be
executed. Both of these examples can be confirmed using the `file`
command.

Besides the possible file types, there are two types of linking:

* __Statically Linked__ - External definitions are copied to the
ELF file. Can be done using the `-static` flag in `gcc`.

* __Dynamically Linked__ - The name of the external modules are saved in
the object file. When the object file is run, the modules are loaded from
the system. By default, `gcc` does dynamic linking.

Just like the preprocessor and assembler stage, there is an program
made specially for the linking stage: `ld`.
