# The Stack

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
