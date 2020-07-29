# Paging

dependencies:
	[Memory](memory.md)

sources:
https://eprint.iacr.org/2013/448.pdf \
http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf \

In Paging, virtual memory is organized by blocks of memory called
__Pages__, each one usually being 4 KB in size (4096 bytes).

Physical memory is also divided in blocks that have the same size
as the __Pages__, called __Frames__.

Mapping the physical to virtual memory is simply done by
determining to which __Frame__ is each __Page__.

Each __Page__ can be associated to any __Frame__, allowing for
a lot of flexibility.

## Page Addressing

It is really simple to address pages.

Each page always is `2^n` bytes in size (`2^12` bytes is 4 KB), so
we can divide a addresses like so:

* The `n` least significant bits of each physical address
define the position of that address inside the page (_offset_).

* The rest of the bits are used to define the page number.

### MMU Translation

So, to translate virtual to physical addresses the MMU does the following:

1. Divide _offset_ from page number.

2. Get the __Frame__ for the specific page number.

3. Build the physical address, by sticking the _offset_ and __Frame__ number
together.

If the MMU sees that the processor is trying to access a page that is not
mapped in the __Active Page Table__, it will generate an interruption
(_Page Fault_) that will be handled by the processor.

## Page Tables

This mapping is done using __Page Tables (PTs)__, where each
entry has the number of a __Page__ and the __Frame__ it belongs to.

Each process has its own __Page Table__.

The MMU has a register called _Page Table Base Register (PTBR)_, which
points to the __PT__ of the current process begin executed. This register
must be updated for each context switch.

And yes, __Page Tables (PTs)__ are also stored in pages themselves,
like everything else in a paginated memory!

### Flags

Each entry in the __Page Table__ has a set of _flags_, for status or
control. Each flag is represented by a bit:

* __Valid__ - Indicates if the page exists in the process addressing space.
If it is 0, any attempt to access the page will raise an interruption
(_Page Fault_).

* __Writable__ - Indicates if the page can accessed for read and write (1)
or just reading (0).

* __User__ - If set (1), user code can access the page, otherwise (0),
only the kernel can access it.

* __Present__ - Indicates if the page is in the RAM (1), or if it is
in a secondary memory (0).

* __Accessed__ - Indicates if the page was accessed recently. It is
activated by the MMU for each access and can be unset by the kernel
when desired. Used for disk paging algorithms.

* __Dirty__ - Set by MMU after a write is performed in the page. Informs
that the page was changed. Also used by disk paging algorithms.

There also can be bits for caching, if it is in the disk, page size
(if more than one is being used), and bits useful for kernel algorithms.

### Multilayer Tables

Data structure used for better management of __Page Tables__.

It is a tree in which all inner nodes are tables that point to
other tables (their children), and leaf nodes just have normal
__Page Tables__.

Each entry in the inner nodes correspond to the value of the bits in
a certain position of the address. The first layer may be indexed by
the number made by the first 10 most significant bits, the second
layer the next 10, and so on.

The benefit of this is that now we don't waste that much memory with
each table: since the first bits in the first layer, lower layers don't
need to repeat these first bits.

### Page Table Caching

Structuring the tables in multilayer tables may reduce the memory being
used, but it makes the access to memory takes longer.

To minimize this, recent page accessed can be kept in a
cache inside the MMU, so it isn't necessary to traverse the multilayer
tables to get them. This cache is called
__Translation Lookaside Buffer (TLB)__.

Each entry in the __TLB__ is indexed by the __Page__ number and has the
__Frame__ number.

Using the __TLB__, the MMU does the following when translating virtual
addresses:

1. Divide _offset_ from page number

2. MMU checks __TLB__ to see if there is a matching entry for the page
number. There is 2 possible outcomes here:

	* __TLB Miss__ - No matching entry is found, a complete search
	must be done.

	* __TLB Hit__ - A matching entry is found! Just use it.

3. The frame number is obtained to build the physical address

4. The frame number is added to the __TLB__ to make future queries faster.

## Page Sharing

There are two reasons for sharing a page between
processes:

* allowing inter-process communication

* reducing memory footprint

### Reducing Memory Footprint

Actually is really common to share pages with this
purpose in mind. Let's look at one example
where this generally happens:

* __content-aware sharing__ - identical pages
are identified by the disk location the pages
are loaded from.

This is the traditional form of sharing in a
OS, used for sharing the text segment of
executable files between processes executing
it and when using shared libraries.

It is implemented in all major OS and has
been suggested for some hypervisors.

* __content-based page sharing__ (or __memory
de-duplication__ ) - system scans the active
memory, identifying and coalescing unrelated
pages with identical contents. 

It is implemented in the VMWare and PowerVM
hypervisors, Linux and Windows.

### Security

The system must protect the contents of the
pages to prevent malicious processes from
modifying the shared contents.

In order to do this, the system maps shared
pages as __copy-on-write__.

* __copy-on-write__ - read operations on these
pages are permitted whereas write operations
cause a CPU trap. The system software, which
gains control of the CPU during the trap, copies
the content of the shared page, maps the copied
page into the address space of the writing process
and resumes the process.
