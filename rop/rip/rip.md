# Overwriting the RIP/EIP

sources:
https://codearcana.com/posts/2013/05/28/introduction-to-return-oriented-programming-rop.html \

When a function is called, a return address is saved in the stack so the
program knows where to take instructions from after the function exits, by
loading this address to the Instruction Pointer.

With `first_buffer_overflow.c` we learned how to overwrite a variable below
the buffer to get access to a part of the program, but if we can overwrite
the RIP (or EIP) we will be able to go to any part of the program
after a function is called.

Overwriting the RIP to redirect the execution flow of the program is the
key concept in Return-Oriented Programming (ROP).

Our "victim" will be the program at `rip.c`:

```
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void impossible_shell(){

	system("/bin/sh");
}

int password_is_correct(char* passwd){

	char buffer[16];
	strcpy(buffer, passwd);

	return !strncmp(buffer, "passwd1", 16);
}

int main(int argc, char* argv[]){

	if( password_is_correct(argv[1]) ){

		printf("PASSWORD IS CORRECT!\n");
		printf("i don't feel like giving you a shell though😝\n");

	} else {

		printf("PASSWORD IS INCORRECT!\n");
		printf("try harder next time\n");
	}
}

```

It is important to note some stuff about the program:

* The function *impossible_shell* is **never** called by the program.
We are going to call it anyway :wink:

* The input is given directly from the command line. If scanf was used
the only difference is that we would use some command line trick to
redirect the desired input to the program. The concepts used for the actual
ROP wouldn't change.

## The Compilation

Just like in `first_buffer_overflow.c`, we will compile with some flags
to allow us to exploit the program easily. We will introduce some new
flags and gradually compile without some of them as we understand how
to bypass the defenses they disable.

## Easiest Cenario

### The Compilation

We will compile using
`gcc rip.c -m32 -no-pie -fno-stack-protector -o rip`:

* `-m32` - compile to 32 bits. The code will still run on 64 bit
architectures. If your compilation throws an error, try installing the
`libc6-dev-i386` package in Linux. 32 bit addresses are smaller and
usually don't contain null bytes, which we can't write to an string.

* `-no-pie` - force to produce position-dependent code. When the code
is position-independent the OS usually puts it in a random location in
memory. We will first execute the program to discover relevant
addresses and then execute again to use them, and without this flag
the addresses would be different between executions.

* `-fno-stack-protector` - disables canary. Canary is a defense technique
against buffer overflow that basically puts a random value in the stack
and check if it changes at some point. If it does is because a buffer
overflow overwrote it and the program is terminated.

* `-o` what comes after this flag will be the name of our executable.

Or you can just use the `Makefile`'s  `first` rule with `make first`.

### Plan of Attack

As stated before, ROP is all about redirecting the flow of execution
by overwriting the return address in the stack. All we need to make
it happen is the address to `impossible_shell` and how far our
buffer is from the return address.

Mind that we are not going to overwrite the return address from `main`,
since the buffer is actually in the `password_is_correct` stack frame.

### Getting r00t

First, launch gdb with `gdb -q rip`. We will use the `info functions`
command in `gdb` to search for `impossible_shell`.

```
info functions impossible_shell
```

Obs .: `info functions` does the search using regex. Just writing
`impossible` would do the job!

You should have an output like this one (the address is probably
different though):

```
All functions matching regular expression "impossible":

Non-debugging symbols:
0x0804849b  impossible_shell
```

We now only need the distance from the buffer to the return address.
To get it, all we really need is to run
`disassemble password_is_correct` to peek under the hood.

We know that our buffer is used as argument for `strcpy` and
`strncmp`,  so we just need to check what address (from the stack)
pushed to the stack before these functions are called.

Indeed, just before `strcpy` is called we have:

```
0x0804852c <+21>:	push   DWORD PTR [ebp+0x8]
0x0804852f <+24>:	lea    eax,[ebp-0x18]
0x08048532 <+27>:	push   eax
```

And just before calling `strncmp`:

```
0x08048540 <+41>:	lea    eax,[ebx-0x1971]
0x08048546 <+47>:	push   eax
0x08048547 <+48>:	lea    eax,[ebp-0x18]
0x0804854a <+51>:	push   eax

```

Obs .: In this case, all the information we just gathered using
`gdb` could be fetched using `objdump -d -M intel-mnemonic rip`.

In both snippets, the common address is **ebp-0x18**. Which means
that after the 18th byte our buffer start writing EBP
(which has 4 bytes) and then our target: EIP (also 4 bytes).

All that is left to do now is write the payload and give it
as the input to the program:

```
./rip `python -c "print(b'A'*0x18 + b'BBBB' + b'\x9b\x84\x04\x08')"`
```
Or, using python3:
```
./rip `python3 -c "__import__('sys').stdout.buffer.write(b'A'*0x18 + b'BBBB' + b'\x9b\x84\x04\x08')"`
```

Here we have 24 (0x18) bytes to fill the space between the EBP
and the start of the buffer, after that we have `'BBBB'` to fill
the EBP, and finally we have the value that will be the new
Return Address.

Obs.: My machine uses little-endian, that's why the address byte
order is backwards. Yours is probably too (most are nowadays)

Running any of the above commands will grant you a shell:

```
$
```

Success! We did it! However, we did it in a really agressive
way. When you exit the shell look what happens:

```
$ exit
Segmentation Fault
```

A Segmentation Fault! Depending on what the program you are exploiting
does this could have some undesired consequences and also somebody could
be monitoring the system for crashing programs! Either way, it is usually
a good idea to make sure your program return the execution flow of the
program to normal after redirecting it.

So, diving deeper into what happen: Why did the `Segmentation Fault`
happened?

When we force the program to go to `impossible_shell`, we aren't using
the proper `call` command. We are just telling the program
"Hey! Go there!" without giving him information how to get back. When
`impossible_shell` ends and uses `leave` and `ret`, two functions that
get previously pushed values from the stack, the `Segmentation Fault`
happens, because just by overwriting the Instruction Pointer we aren't
pushing the proper values that `leave` and `ret` will fetch from the
stack later.
