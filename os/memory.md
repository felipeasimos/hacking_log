# Memory

dependencies:
	[Operating System](os.md)
	[Processes](processes.md)

sources:
http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf
http://www.nastooh.com/teaching/William_Stallings_Operating_Systems_7th_Edition.pdf
http://digitalthinkerhelp.com/virtual-memory-in-os-operating-system-demand-paging/

There are many types of memory in a computing system, but all with
the same purpose: store data.

Memories have different technologies, capacities, speed, energy consumption,
cost per byte stored and volatibility. These characteristics allow us to
define a __memory hierarchy__:

![Memory Hierarchy](https://i0.wp.com/www.vlsifacts.com/wp-content/uploads/2015/07/Computer-Memories.png)

As you can see, speed and cost increase as we go higher (it is not in the image
but energy consumption does too!) and capacity decreases.

The "speed" here means the time it takes to read/write a byte.

## RAM Memory

RAM (Random Access Memory) is the primary memory in a cumpter system. Each
byte in the RAM memory has an address, which is used to access it.

Usually the execution of programs don't happen directly on the physical
memory (except for really some embedded or old systems). Modern systems
use virtual memory and the concept of addressing space to abstract the
access to memory.

## Addressing Space

When we say a computer has 64 bit or 32 bits, we are talking about
the number of bits that make up an address in that system.
Every address i used to reference a byte (8 bits). That means:

* 32 bit architectures can address up to 4 GBytes.

* 64 bit architectures can address up to 256 Terabytes.

The __addressing space__ is the set of addresses that can be used
in a system, given the number of bits.

The __addressing space__ can be much larger than the actual memory
capacity in the system. That means some addresses can be invalid.

## Virtual Memory

To abstract and simplify the access to memory, modern systems use
__virtual memory__, where there are two kinds of addresses:

* __physical addresses (real)__  - Addresses of physical bytes in
the memory. These addresses are defined by the memory capacity in
the machine.

* __logical addresses (virtual)__ - Addresses used by processes,
OS and processor. These addresses are defined according to the
addressing space.

When executing, processes can only "see" __virtual memory__. During
the program execution, the processor generates __logical addresses__
to access, which will be them translated to __physical addresses__
in the RAM memory. This translation is done by the MMU (Memory
Management Unit), which in most modern processor have built-in.

The MMU intercepts __logical addresses__ emitted by the processor
and converts them to corresponding __physical addresses__, so it
can accessed.

In case an access to a __logical address__ isn't possible (it
may not be associated to a __physical address__ for example), the
MMU generates an interruption to notify the processor.

![MMU Scheme](https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Ftse1.mm.bing.net%2Fth%3Fid%3DOIP.e-f2hw7oMhl1ObZPpmRZoAHaEK%26pid%3DApi&f=1)

To ensure each process memory region can't be accessed by others,
the kernel has distinct rules for the address translation for
each process and reconfigures the MMU for each context switch.

More sofisticated MMUs also have the definition of access
permissions for memory areas. This feature allow every
process to have different permissions for certain areas
in its memory region.
