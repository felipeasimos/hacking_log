# Registers

sources:
http://www.eecg.toronto.edu/~amza/www.mindsec.com/files/x86regs.html \
https://www.assemblylanguagetuts.com/x86-assembly-registers-explained/ \
https://codearcana.com/posts/2013/05/21/a-brief-introduction-to-x86-calling-conventions.html
https://www.ret2rop.com/2018/08/cpu-memory-and-buffer-overflow.html

## Layout and Syntax

The register name is what we use in assembly to reference it,
but registers overlap! For example the EAX register (32 bit) is the
physical second half of the RAX (64 bit). Not all registers work
this way though, some don't have an 16 or 8 bit counterpart.

Modern 64 bit machines use 64 bit registers. Most of these registers
are basically new versions of the 32 bit ones, that "keep" the 32 bit
originals inside them. There are also new registers, numbered from 8 to
15 (r8, r9, r10, r11 ... r15). These registers have some different
syntax for accessing their 32, 16 and 8 bit counterparts.

```
 ________________________________________________________________
|________________________________________________________________| 64 bit register
 ________________________________
|________________________________| 32 bit register
 ________________
|________________| 16 bit register
 ________
|________| first 8 bit register
	 ________
	|________| second 8 bit register
```

### 64 bit register

***Syntax Rule***: Starts with an 'R'.

***Layout***: Only present in 64-bit architectures. It is the
biggest register.

Ex:
> RAX, RBX, RCX, RIP, R15

### 32 bit register

***Syntax Rule***

* For original 32 bit registers (e\_x)

	* Starts with an 'E'

* For added 32 bit registers (r\_d)

	* Add an 'D' suffix

***Layout***
In a 64 bit architecture, it is the physical first half of an 64 bit register.
In a 32 bit architecture it is the biggest register.

Ex:
> EAX, EBX, ECX, EIP, R15D

### 16 bit register

***Syntax Rule***

* For original 16 bit registers (\_x)
	
	* No prefix

* For added 16 bit registers (r\_w)

	* Add an 'w' as suffix

***Layout***: It is the physical first half of an 32 bit register

Ex:
> AX, BX, CX, IP, R4W

### First 8 bit register (lower half)

***Syntax Rule***:

* For original 8 bit registers (\_l)

 	* Switch the 'X' in the 16 bit counterpart name for
	an 'L' (meaning low byte, since this register access the least
	significant bytes of its 16 bit counterpart)

* For added 8 bit registers (r\_b)

	* Switch the 'X' in the 16 bit counterpart name for an 'B'

***Layout***: It is the physical first half of an 16 bit register

Ex:
> AL, BL, CL, R5B

### Second 8 bit register (higher half) (only available for original registers)

***Syntax Rule***: Switch the 'X' in the 16 bit counterpart name for
an 'H' (meaning high byte, since this register access the most
significant bytes of its 16 bit counterpart)

***Layout***: It is the physical second half of an 16 bit register

Ex:
> AH, BH, CH

## Types

### To Save or Not Save?

Registers can be divided in two groups: *caller save* registers and
*callee save* registers.

#### Caller Save Registers

Also called volatile registers. They are `rax`/`eax`, `rcx`/`ecx`,
`rdx`/`edx`, `rdi`/`edi`(in linux), `rsi`/`esi`(in linux), `r8`,
`r9`, `r10` and `r11`. They can be used freely inside a function,
but are not guaranteed to keep the same value after a function
calling (because other functions can also use them freely), so
you must save them before calling a function to use the value
later (by pushing their values to stack and popping it back after
the `call`).

#### Callee Save Registers

Also called non-volatile registers. They are `rbx`/`ebx`,
`rbp`/`ebp`, `rdi`/`edi`(in windows), `rsi`/`esi`(in windows),
`rsp`/`esp`, `r12`, `r13`, `r14`, `r15`. Their original
values must be saved before using them, and their original
values must be restored before before the current function
return.

### Grouping by Purpose

Another way to group registers, is by their purpose.

#### Data Registers

Holds data about for operations. They are also called general purpose
registers.

64 bit |32 bit | 16 bit | 8+8 bit
-------|-------|--------|--------
RAX | EAX | AX | AL+AH
RBX | EBX | BX | BL+BH
RCX | ECX | CX | CL+CH
RDX | EDX | DX | DL+DH

* ***Accumulator Register (RAX|EAX|AX|AL|AH)***

	* The preferred register to be used in I/O, arithmetic, logic and data transfer
	operations, like multiplication and division.

	* store return value of functions

	* volatile

* ***Base Register (RBX|EBX|BX|BL|BH)***

	* Hold the address of the base storeage location from where the data were
	stored continuously

	* It is used to find the data that is required

* ***Count Register (RCX|ECX|CX|CL|CH)***

	* The preferred register to be used as a loop counter

	* Every counting-related instruction use it

	* volatile

* ***Data Register (RDX|EDX|DX|DL|DH)***

	* I/O operations

	* The preferred register to be used in division and multiplication
	of large numbers

	* volatile

### Address Registers

Usually store the memory address from where data will be fetched into the CPU
or address to which data will be sent to or stored in. They are divided in
three parts:

Segment | Pointer | Index
--------|---------|-------
CS | SP | SI
DS | BP | DI
SS | |
ES | |

#### Segment Registers

These registers store addresses of different segments of the executable. By
using them and an offset we can point to any address in our program.

* ***Code Segment (CS)***

	* Store the initial address of the code segment, which
	store all instructions our program will be using

* ***Data Segment (DS)***

	* Store the initial address of the data segment

* ***Stack Segment (SS)***

	* Points to the segment used by the stack

	* Totally different from the Stack Pointer

* ***Extra Segment (ES, FS, GS)***

	* Used to point to a second data segment, like video memory

#### Pointer Registers

These registers point to memory location crucial to the program. Buffer
Overflows attacks use them a lot.

* ***Stack Pointer (SP)***

	* Used in combination with the Stack Segment(SS) for accessing the
	stack segment.
	
	* Points to the top of the stack

	* Used to reference local variables

* ***Base Pointer (BS)***

	* Also Used in combination with the Stack Segment(SS).
	
	* Used to access data in the stack

	* Points to the base of the stack

	* Used to save the position Stack Pointer(SP) pointed to when
	function was called

* ***Instruction Pointer (IP)***

	* Used in combination with the Code Segment(CS)

	* Sometimes called as index pointer

	* Store the offset address of the next instruction to be executed

### Index Registers

* ***Source Index (SI)***

	* Used to point to memory locations in the data segment addressed
	by the Operating System

	* By incrementing the contents of SI, we can easily access
	consecutive memory locations

	* Used for string and memory array copying

	* Source index for string operations

* ***Destination Index (DI)***

	* Perform the same functions as SI

	* Used for memory array setting

	* For string operations it is used to access memory locations
	addressed by ES

	* Destination index for string operations

### EFLAGS Register

Store the current status of the processor. Each bit in this register
is called flag and has an special meaning

Status Flags | Control Flags
-------------|---------------
CF    SF | TF
PF    OF | IF
AF    ZF | DF

#### Status Flags

Used by the processor to reflect the result of an operation.

NAME | SYMBOL | BIT
-----|--------|-----
Carry Flag | CF | 0
Parity Flag | PF | 2
Auxiliary Carry Flag | AF | 4
Zero Flag | ZF | 6
Sign Flag | SF | 7
Overflow Flag | OF | 11

#### Control Flags

NAME | SYMBOL | BIT
-----|--------|-----
Trap Flag | TF | 8
Interrupt Flag | IF | 9
Direction Flag | DF | 10
