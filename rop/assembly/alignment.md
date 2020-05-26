# Alignment

sources:
https://www.youtube.com/watch?v=OKjOZBaKlOc \
https://en.wikipedia.org/wiki/Data_structure_alignment \
https://stackoverflow.com/questions/19128291/stack-alignment-in-x64-assembly \
https://stackoverflow.com/questions/5538079/why-alignment-is-16-bytes-on-64-bit-architecture \
https://gcc.gnu.org/legacy-ml/gcc-help/2010-01/msg00113.html \
https://stackoverflow.com/questions/4175281/what-does-it-mean-to-align-the-stack \
https://forum.nasm.us/index.php?topic=1689.0 \
https://stackoverflow.com/questions/612443/why-does-the-mac-abi-require-16-byte-stack-alignment-for-x86-32

The stack is the most important structure in the context of binary
exploitation but even the most simple programs can generate
unexpected assembly at first glance. Simple programs like
`simple.c`

## Data Alignment

 Name | Size | Alignment
------|------|-----------
BYTE  | 1 byte | Anywhere
WORD | 2 bytes | Even addresses
DWORD | 4 bytes | Adresses multiple of 4
QWORD | 8 bytes | Adresses multiple of 8

When reading/writing memory, the table above should be followed,
which means that any continuous region of memory must be
read/write at/from an address that is a multiple of its size.

This is a good practice because it is easier to index the
desired memory, and thus faster.

This has some direct consequences in the stack layout. For
example, imagine you have an `int` and a `long int` in the stack.
The `int` has 4 bytes, and so it correspond to a DWORD, and
the `long int` has 8 bytes, like a QWORD.

The stack can't have this layout (remember that the stack
grows downwards):

Bottom (higher addresses) |
------------------------|
`int` (4 bytes) at 0x10 |
`long int` (8 bytes) at 0x14 |

Total stack size: 12 bytes

Following the first table, we can see the `long int` can't be
at an address that is not a multiple of 8. Correcting the 
stack results in this layout:

Bottom (higher addresses) |
------------------------|
`int` (4 bytes) at 0x10 |
Padding (4 bytes) |
`long int` (8 bytes) at 0x14 |

Although the Padding section won't be used, it is a consequence
of how the memory is 

Total stack size: 16 bytes

## Stack Alignment

There are some differences between the theory and pratice in the stack
frame lifetime, one of them is **Stack Alignment**. What this means is
that the Stack Pointer will point to an address that is multiple of a
number for optimization purposes. By default this number is 16,
which is a requirement from the ABI (application binary interface),
probably due to SSE instructions (streaming SIMD extensions) having a
bad performance without this alignment.

Of course, the stack is a data structure that can constantly change,
but the usual calling convention used says the stack must be aligned
before a function call. This means that when debugging, if you pause
the execution right before the `call` command, the stack should be
aligned.

The `gcc` compiler follow this alignment rule, but it can be changed with
the flag `-mpreferred-stack-boundary=n`, where `n` is a power of 2. 2 to
the power of `n` will be the alignment used (default power is 4, hence the
16 byte alignment).

When analysing a binary in the wild though, be aware: not all programs
are compiled equally. Alignment settings can be changed and it is not
always clear what the compiler have done under the hood. That's why
tools like `gdb` are so necessary.

One thing we can be sure at least is: for 32 bit programs the minimal
alignment is of 4 bytes, since this is the size of an 32 bit address,
and the same goes for 64 bit, where the minimal alignment setting is
8 bytes. You can test this by trying to compile a program with the
flags `-m32` and `-mpreferred-stack-boundary=1` or `-m64` with
`-mpreferred-stack-boundary=2` or `-mpreferred-stack-boundary=3`.

For example, in the `simple.c` file we have the following code:

```
#include <stdio.h>

int main(){

	int x=3;
	int y=4;

	printf("x: %p\n", &x);
	printf("y: %p\n", &y);

	return 0;
}
```

### Pratical Example

If we call `objdump -d -M intel-mnemonic simple` after compilation
(with `gcc simple.c -m32 -fno-stack-protector -o simple`), we get
the assembly code. If we take a look at the `main` function we
will find the lines:

```
 5bd:	c7 45 f4 03 00 00 00 	mov    DWORD PTR [ebp-0xc],0x3
 5c4:	c7 45 f0 04 00 00 00 	mov    DWORD PTR [ebp-0x10],0x4
```

Where `x` is being given the value of `3` in the first line and `y` is
being given the value of `4` in the second.

By launching `gdb`, we can pause just before a function call and see
that ESP's hexadecimal value ends with a 0 (it is a multiple of 16).

These are the only local variables in our program and yet, if we draw the
layout of the stack using the addresses we get the following:

rbp-0x10 = 16 \
rbp-0xc = 12

Higher Addresses (Bottom of the stack) |
---------------------------------------|
byte 1 (empty, where RBP points to) |
byte 2 (empty) |
byte 3 (empty) |
byte 4 (empty) |
byte 5 (empty) |
byte 6 (empty) |
byte 7 (empty) |
byte 8 (empty) |
byte 9 (fourth `x` byte) |
byte 10 (third `x` byte) |
byte 11 (second `x` byte) |
byte 12 (first `x`byte) |
byte 13 (fourth `y` byte) |
byte 14 (third `y` byte) |
byte 15 (second `y` byte) |
byte 16 (first `y` byte |
Rest of the frame |
Lower Addresses (Top of the stack) |

As you can see we have 8 bytes in the bottom at the top
just so we can have ESP pointing to a multiple of 16.

Analysing a `simple.c` compiled to 64 bit yield similar
results.