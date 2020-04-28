# Assembly Basics

sources:
http://www.cs.virginia.edu/~evans/cs216/guides/x86.html \
https://stackoverflow.com/questions/1658294/whats-the-purpose-of-the-lea-instruction

NOTE: based on the intel flavor syntax (`set disassembly-flavor intel`)

## Square Brackets

To get a value in an address use square brackets ([]) to deference it.

`push DWORD PTR [ecx-0x4]`

To reference a space in memory too:

`mov DWORD PTR [ebp-0xc], 0x1`

## Memory Size

When we are dealing with values in a given address, we should say how much
memory we are manipulating.

For example:

`mov DWORD PTR [ebp-0xc], 0x1`

If we just had `mov [ebp-0xc], 0x1` there would be no way to know how much
space in memory `0x1` should occupy. If it is 1 byte, we should write
7 zeros and then a 1, if it is 2 bytes we should write 8 zeros in the
next byte (ebp-0xb) and then 7 zeros and a 1 in ebp-0xc.

The syntax is the following:

* BYTE - 8 bits, 1 byte
* WORD - 16 bits, 2 byte
* DWORD - 32 bits, 4 byte
* QWORD - 64 bits, 8 byte

## Commands

A list with a short description of some common assembly commands:

* `push` - write the given value to the top of the stack, decrementing the
Stack Pointer (since the stack grows downwards)

* `pop` - take out the value from the top of the stack and write it to
the given operand, incrementing the Stack Pointer (since the stack growns
downwards)

* `lea` - (load effective address) give the address of the right argument
to the left argument. So when the right argument is an address that is
being deferenced by square brackets, it will be referenced again by `lea`
and the address wil be given to the left argument.

* `mov` - give the value at the right argument to the left argument. If one
of the arguments is an address in square brackets, the size of the memory
to be copied from the address should be given by "<size> PTR" before the
reference. `size` can have the values shown in the
[Memory Size](#memory-size) section.

* `cmp` and `jmp` - `cmp` takes two operands and subtract their values,
without changing them, to compare them (setting some status flag). `jmp`
is just an unconditional jump to another part of the code, but is
variations, like `je`, `jne` and `jg` (there are orders) check the status
flags to decide to make a jump or not. Every `jmp`-like command takes in
one argument, which is the address they will jump to if conditions are met.
