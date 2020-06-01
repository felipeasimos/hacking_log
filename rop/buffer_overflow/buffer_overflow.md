# Buffer Overflow

dependecies:
	[Stack](../assembly/stack.md) \
	[Registers](../assembly/registers.md) \
	[Basics](../assembly/basics.md) \
	[Alignment](../assembly/alignment.md) \
	[Endianness](../assembly/endianncess.md)

sources: https://sourceware.org/gdb/onlinedocs/gdb/Continuing-and-Stepping.html

In programming, a buffer is just a space in memory meant to store some data.
To do a Buffer Overflow is to write more data to a buffer than it was
designed to hold. By doing so, we write to memory locations we weren't
supposed to and thus alter the program expected execution.

Buffer Overflow vulnerabilities are accidentally created by developers
when using a low-level programming language, like C/C++ or assembly.
High-level programming languages like Javascript or Python check if the
data being written don't exceed the buffer size and the developers using
them usually don't need to worry about a possible Buffer Overflow. However,
this check makes execution slower when compared to programs made with a
low-level programming language. The trade-off between security and speed
must be taken into account when choosing which language to use.

## A Simple Program

We will use the following program, that can be seen in the
`first_buffer_overflow.c`:

```
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){

	int passcheck=0;
	char password[16];

	//get user input
	printf("Type in the password: ");
	scanf("%s", (char*)&password);

	//check if it is correct
	if( strcmp(password, "password1") ){
	
		printf("\nACCESS DENIED\n");

	} else {

		printf("\nACCESS ALLOWED\n");
		passcheck=1;
	}

	//get shell
	if( passcheck ){

		system("/bin/sh");
	}

	return 0;
}
```

The program writes the input to a buffer and compare it to a constant
string "password1". The variable `passcheck` changes value and is
ultimately the one responsible for allowing us to get a shell.

The program behaves like so when we give it `hello` as the password:

```
Type in the password: hello

ACCESS DENIED
```

And like so when we give it `passsword1`, the correct password:

```
Type in the password: password1

ACCESS ALLOWED
$ 
```
### Compilation

The outputs above should match yours if you sucessfully compiled the
program. But in most cases you won't be able do a buffer overflow with
your usual compilation options.

In the past, doing a buffer overflow, even in a not controlled environment,
was simpler. Nowadays we have more defenses against it (fortunately) and
more advanced attack techniques. Before we get to these more complex
concepts though, we need to get a grip with the fundamentals, and therefore
change some compilation settings to lower the defenses and get a basic
buffer overflow going without much hassle.

We are going to use the `gcc`'s `-fno-stack-protector` and `-m32` options
to compile `first_buffer_overflow.c`. If you do it manually in your
terminal it should look like this:

```
gcc -fno-stack-protector -m32 first_buffer_overflow.c -o first_buffer_oveflow
```

Let's break it down.

* `-fno-stack-protector` disables canary. Canary is a defense technique
against buffer overflow that basically puts a value in the stack and check
if it changes at some point. If it does is because a buffer overflow
overwrote it and the program is terminated.

* `-m32` compile to 32 bits. The code will still run on 64 bit architectures.
If your compilation throws an error, try installing the `libc6-dev-i386`
package in Linux. 

* `-o` what comes after this flag will be the name of our executable.

The file can also be compiled with `make`.

### Getting the Shell

We need to overwrite the `passcheck` variable and give it a non-zero value.
But the only thing in our control is the `password` buffer. By now, 
we already know a buffer overflow consists of writing more data than
we are supposed to the stack, so let's try it!

```
Type in the password:  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA 

ACCESS DENIED
Segmentation fault (core dumped)
```

Well, looks like that didn't work! What probably happened is that we
attempted to write to memory outside the `first_buffer_overflow` process
and the operating system killed out program to prevent it.

Let's think how the memory stack in our program is layed out to do this
with more precision. What is going on under the hood: 

Lower Addresses (top of the stack) |
-----------------------------------|
password (16 bytes) | 
passcheck (4 bytes) |
Higher Addresses (bottom of the stack) |

In our attempt to write too much A's we wrote way past the
`passcheck` fourth byte, but all we needed to do was write a non-zero value
to the first one. So let's write 17 A's and see what happens:

```
Type in the password: AAAAAAAAAAAAAAAAA

ACCESS DENIED
$ 
```

We get a shell! But notice that we still got `ACCESS DENIED`. That's
because the logic that choose wheter to say `ACCESS ALLOWED` or
`ACCESS DENIED` only check if the string given is "password1" and doesn't
care about the value of `passcheck`.

When we type `exit` the shell exits gracefully, no errors! But we could
get a shell **and** a segmentation fault if we write to the return address
saved in the stack.

Lower Addresses (top of the stack) |
-----------------------------------|
password (16 bytes) |
passcheck (4 bytes) |
saved EBP  (4 bytes)|
return address (4 bytes)|
Higher Addresses (bottom of the stack)

So if we write beyond 16 + 4 + 4 = 24 bytes, we get are writing to the
return address. If we write garbage to it (our army of A's will do) we'll
probably have an address that doesn't belong to this process.

Writing 25 A's:

```
Type in the password: AAAAAAAAAAAAAAAAAAAAAAAAA

ACCESS DENIED
$ exit
Segmentation fault (core dumped)
```

If we write just 24 A's, that's also enough to write to the return address,
since `scanf` will add a null terminator to the end of our string:

```
Type in the password: AAAAAAAAAAAAAAAAAAAAAAAA

ACCESS DENIED
$ exit
Segmentation fault (core dumped)
```

If we write less than 25 bytes, no segmentation fault happens. But from
byte 20 to 23 we are writing to the saved EBP, the base pointer, and that
could have affected the execution of the program and led to a segmentation
fault. In our case it didn't, but be aware of the possibility.

### Finding the Vulnerability Without Source Code

We already know what a buffer overflow is, but how would you identify it
if we didn't have access to the source code?

To get our questions answered, we'll use `gdb`. `gdb` stands for
'GNU Debugger' and it is a tool for debugging executables.

To launch our program with `gdb`, type `gdb -q ./first_buffer_overflow` in the
terminal (the `-q` option is to make it less verbose). A prompt like this
one should appear:

```
Reading symbols from ./first_buffer_overflow...(no debugging symbols found)...done.
(gdb)
```

`gdb` has bunch of commands. Here are the ones we are going to use in this
section:

* `disassemble <point>` (or `disass <point>`) - show the assembly code from
the given `point`. The `point` can be an address (will show code of the
function which the address belongs to) or a function name.

* `set disassembly-flavor <flavor>` - show assembly code in the given
syntax. The options are `att` (default) and `intel`.

* `break <point>` - create breakpoint at a `point`.
Just like `disassemble`, `point` can be an address or a function name
(will break as soon as it is called). The program execution will stop
at the break point if it hits it.

* `run <args>` - run the program. `args` are the command line arguments.

* `ni` - go to next assembly instruction and pause. If it is function call,
wait for it to return to pause.

Let's find the information we had in the source C code in the assembly
code:

* `passcheck` variable is responsible for giving us a shell (though we
won't know the variable name from assembly, but that doesn't matter)

* Where our buffer is at

* Where the `passcheck` is

* How much we need to write to get to the `passcheck variable`

From here on, let's pretend we don't know nothing about the source code.
The only thing we know is how the program behaves when a wrong password
is given.

First, we need to set the syntax to our preference. If you like `att` there
is no need to do anything, if you prefer intel (like i do) run this:

```
set disassembly-flavor intel
```

And get the assembly code of the `main` function like so:

```
disassemble main
```

Note that we could not find all the information inside the main function,
but the other functions would be called from it, and we could investigate
further by using `dissasemble <function name>`.

The assembly code of `main` looks like so (in my machine, and using intel
syntax, yours may differ slightly):

```
Dump of assembler code for function main:
   0x080484fb <+0>:	lea    ecx,[esp+0x4]
   0x080484ff <+4>:	and    esp,0xfffffff0
   0x08048502 <+7>:	push   DWORD PTR [ecx-0x4]
   0x08048505 <+10>:	push   ebp
   0x08048506 <+11>:	mov    ebp,esp
   0x08048508 <+13>:	push   ecx
   0x08048509 <+14>:	sub    esp,0x24
   0x0804850c <+17>:	mov    DWORD PTR [ebp-0xc],0x0
   0x08048513 <+24>:	sub    esp,0xc
   0x08048516 <+27>:	push   0x8048620
   0x0804851b <+32>:	call   0x80483a0 <printf@plt>
   0x08048520 <+37>:	add    esp,0x10
   0x08048523 <+40>:	sub    esp,0x8
   0x08048526 <+43>:	lea    eax,[ebp-0x1c]
   0x08048529 <+46>:	push   eax
   0x0804852a <+47>:	push   0x8048637
   0x0804852f <+52>:	call   0x80483e0 <__isoc99_scanf@plt>
   0x08048534 <+57>:	add    esp,0x10
   0x08048537 <+60>:	sub    esp,0x8
   0x0804853a <+63>:	push   0x804863a
   0x0804853f <+68>:	lea    eax,[ebp-0x1c]
   0x08048542 <+71>:	push   eax
   0x08048543 <+72>:	call   0x8048390 <strcmp@plt>
   0x08048548 <+77>:	add    esp,0x10
   0x0804854b <+80>:	test   eax,eax
   0x0804854d <+82>:	je     0x8048561 <main+102>
   0x0804854f <+84>:	sub    esp,0xc
   0x08048552 <+87>:	push   0x8048644
   0x08048557 <+92>:	call   0x80483b0 <puts@plt>
   0x0804855c <+97>:	add    esp,0x10
   0x0804855f <+100>:	jmp    0x8048578 <main+125>
   0x08048561 <+102>:	sub    esp,0xc
   0x08048564 <+105>:	push   0x8048653
   0x08048569 <+110>:	call   0x80483b0 <puts@plt>
   0x0804856e <+115>:	add    esp,0x10
   0x08048571 <+118>:	mov    DWORD PTR [ebp-0xc],0x1
   0x08048578 <+125>:	cmp    DWORD PTR [ebp-0xc],0x0
   0x0804857c <+129>:	je     0x804858e <main+147>
   0x0804857e <+131>:	sub    esp,0xc
   0x08048581 <+134>:	push   0x8048663
   0x08048586 <+139>:	call   0x80483c0 <system@plt>
   0x0804858b <+144>:	add    esp,0x10
   0x0804858e <+147>:	mov    eax,0x0
   0x08048593 <+152>:	mov    ecx,DWORD PTR [ebp-0x4]
   0x08048596 <+155>:	leave  
   0x08048597 <+156>:	lea    esp,[ecx-0x4]
   0x0804859a <+159>:	ret    
End of assembler dump.
```
First interisting thing to notice is that there is a call to `system`.
This could easily let us get a shell if can call it (By just analysing
`main` we don't know that it already does)

Second thing to notice is the control flow of the program. We can have
an idea of what areas of the code depend on a condition to be
executed by seeing where `cmp` and variations of the `jmp` command
appear.

With `0x08048578 <+125>:   cmp    DWORD PTR [ebp-0xc],0x0` we can
infer that a 4 byte variable in the stack (we know it is in the
stack because `ebp` is used to reference it) is compared to `0`.

The next line `0x0804857c <+129>:   je     0x804858e <main+147>` tell
us that if in the previous line the `cmp` command concludes that
the local variable is equal to zero, this jump is going to happen
and `system` won't be called (`je` means "jump if equal"), since we
will jump to an address after it and there is no jump afterwards to
take us back. Conclusion: **this local variable is responsible for
giving us access to `system`** and **it is located at ebp-0xc**.

Third thing to notice, since we are trying to execute a buffer
overflow, is how the local variables are arranged. A good way to get
a sense of it is by reading the lines in which variables in the stack
are addressed. If we look for lines using `ebp` and `esp` inside
square brackets we will find these:

```
   0x080484fb <+0>:     lea    ecx,[esp+0x4]

   0x0804850c <+17>:    mov    DWORD PTR [ebp-0xc],0x0

   0x08048526 <+43>:    lea    eax,[ebp-0x1c]

   0x0804853f <+68>:    lea    eax,[ebp-0x1c]

   0x08048571 <+118>:   mov    DWORD PTR [ebp-0xc],0x1
   
   0x08048578 <+125>:   cmp    DWORD PTR [ebp-0xc],0x0

   0x08048593 <+152>:   mov    ecx,DWORD PTR [ebp-0x4]
```

Just before calling `scanf` the value of `eax` is pushed to the stack,
which means it is an argument to the function call. A pointer is also
pushed and since it is written as a literal, safe to say it is a
constant string (commonly used with scanf). Before that, the address
[ebp-0x1c] is pushed to `eax`. This means that **[ebp-0x1c] is where
our buffer is**.

What we have: ebp-0xc is where the 4 byte variable we want to
overwrite with a non-zero value is and ebp-0x1c is where our buffer
is.

0xc = 12
0x1c = 28

28 - 12 = 16

Finally, we can conclude that the buffer will start overwriting
the variable at the 17th byte.
