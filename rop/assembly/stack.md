# The Stack

sources:
https://stackoverflow.com/questions/19128291/stack-alignment-in-x64-assembly \
https://stackoverflow.com/questions/5538079/why-alignment-is-16-bytes-on-64-bit-architecture \
https://gcc.gnu.org/legacy-ml/gcc-help/2010-01/msg00113.html \
https://stackoverflow.com/questions/4175281/what-does-it-mean-to-align-the-stack \
https://forum.nasm.us/index.php?topic=1689.0

It is the data structure used to do memory management in a program. It
grows downwards (towards lower addresses). \
The top of the stack is
indicated by the RSP (in 64 bit architecture) or the ESP (in 32 bit
architecture). \
The base of the stack is indicated by the RBP (in 64 bit architecture) or
the EBP (in 32 bit architecture).

Everytime a `pop` command is called, the RSP/ESP is automatically
incremented (since the stack shrinks upwards) and when the `push` command
is called, the RSP/ESP is automatically decremented (since the stack grows
downwards).

The important thing to know is that the RSP/ESP always points to the top
of the stack.

## Function Call

When a function is called (including main), the following happens:

1. The parameters of the function are pushed to the stack (nothing happens
if there are no parameters)

**RBP/EBP** = somewhere below Parameter 1 (in a higher address) \
**RSP/ESP** = `Parameter 3` address in the stack

Parameter 3 | <- **RSP/ESP**
------------|---------------
Parameter 2 |
Parameter 1 |

2. The address of the instruction that will be executed after the
function is pushed to the stack (the address right after the one the 
Instruction Pointer (RIP/EIP) is pointing to)

**RBP/EBP** = somewhere below Parameter 1 (in a higher address) \
**RSP/ESP** = `Return Address` address in the stack

Return Address | <- **RSP/ESP**
---------------|---------------
Parameter 3 |
Parameter 2 |
Parameter 1 |

3. Push the value of the Base Pointer (RBP/EBP) to the stack

**RBP/EBP** = somewhere below `Parameter 1` (in a higher address) \
**RSP/ESP** = `RBP/EBP value` address in the stack

RBP/EBP value  | <- **RSP/ESP**
---------------|----------------
Return Address |
Parameter 3 |
Parameter 2 |
Parameter 1 |


4. `mov ebp, esp` (copy the value from RSP/ESP to RBP/EBP, so that now both
RSP/ESP and RBP/EBP are pointing to the value of RBP/EBP that was pushed to
the stack)

**RBP/EBP** = `previous RBP/EBP value` address in the stack \
**RSP/ESP** = `previous RBP/EBP value` address in the stack

previous RBP/EBP value | <- **RSP/ESP** and **RBP/EBP**
---------------|----------------------------------------
Return Address |
Parameter 3 |
Parameter 2 |
Parameter 1 |

5. Change RSP/ESP value, so that it points to the lowest address our stack
will reach.

**RBP/EBP** = `previous RBP/EBP value` address in the stack \
**RSP/ESP** = Somewhere above `previous RBP/EBP value` address 
in the stack (in a lower address)

previous RBP/EBP value | <- **RBP/EBP**
---------------|----------------------------------------
Return Address |
Parameter 3 |
Parameter 2 |
Parameter 1 |

6. Execute the function's code (nothing in our scheme here changes)

**RBP/EBP** = `previous RBP/EBP value` address in the stack \
**RSP/ESP** = Somewhere above `previous RBP/EBP value` address in 
the stack (in a lower address)

previous RBP/EBP value | <- **RBP/EBP**
---------------|----------------------------------------
Return Address |
Parameter 3 |
Parameter 2 |
Parameter 1 |

7. At the end of the function execution, the `leave` command is called.
What it does is copy RBP/EBP's value to RSP/ESP (so now the top of the
stack is the previous value of RBP/EBP we pushed on step `3`).

**RBP/EBP** = `previous RBP/EBP value` address in the stack \
**RSP/ESP** = `previous RBP/EBP value` address in the stack

previous RBP/EBP value | <- **RSP/ESP** and **RBP/EBP**
---------------|----------------------------------------
Return Address |
Parameter 3 |
Parameter 2 |
Parameter 1 |

8. The `leave` command didn't end its execution in the last step. It also
`pop` the previous value of RBP/EBP from the stack back to the RBP/EBP
register.

**RBP/EBP** = `previous RBP/EBP value` (somewhere below `Parameter 1`,
in a higher address) \
**RSP/ESP** = `Return Address` address in the stack

Return Address | <- **RSP/ESP**
---------------|-----------------
Parameter 3 |
Parameter 2 |
Parameter 1 |

9. The `ret` command is called. What it does is pop the Return Address to
RIP/EIP so that the next instruction to be executed is the one we pushed
to stack on step `2`

**RBP/EBP** = `previous RBP/EBP value` \
**RSP/ESP** = `Parameter 3` address in the stack

Parameter 3 | <- **RSP/ESP**
------------|--------------
Parameter 2 |
Parameter 1 |

10. If parameters were pushed to the stack, the next instructions are `pop`
commands that take them out.

## 64 Bit Caviat

There are big differences for compiling a program for 32 bit and for 64
bit. A change that usually get in the way of the usual buffer overflow
logic is **Stack Alignment**. What this means is that the Stack Pointer
will always point to an address that is multiple of 16. This happens is
requirement of the ABI (application binary interface), probably due to
SSE instructions (streaming SIMD extensions) requiring this alignment.

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

If we call `objdump -d -M intel-mnemonic simple` after compilation, we get
the assembly code. If we take a look at the `main` function we will find
the lines:

```
  4005ad:	c7 45 f0 03 00 00 00 	mov    DWORD PTR [rbp-0x10],0x3
  4005b4:	c7 45 f4 04 00 00 00 	mov    DWORD PTR [rbp-0xc],0x4
```

Where `x` is being given the value of `3` in the first line and `y` is
being given the value of `4` in the second.

These are the only local variables in our program and yet, if we draw the
layout of the stack using the addresses we get the following:

0x10 = 16 \
0xc = 12

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
byte 9 (fourth `y` byte) |
byte 10 (third `y` byte) |
byte 11 (second `y` byte) |
byte 12 (first `y`byte) |
byte 13 (fourth `x` byte) |
byte 14 (third `x` byte) |
byte 15 (second `x` byte) |
byte 16 (first `x` byte, where RSP points to) |
Lower Addresses (Top of the stack) |

As you can see we have 8 bytes that are there just so we can have RSP
pointing to a multiple of 16. If `x` and `y` occupied 17 bytes, then
the RSP would be 32 bytes apart from the RBP.
