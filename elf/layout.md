# ELF File Layout

sources:
https://intezer.com/blog/research/executable-linkable-format-101-part1-sections-segments/ \
https://en.wikipedia.org/wiki/Executable_and_Linkable_Format \
https://linux-audit.com/elf-binaries-on-linux-understanding-and-analysis/ \
https://unix.stackexchange.com/questions/132036/why-does-readelf-show-system-v-as-my-os-instead-of-linux \
https://refspecs.linuxbase.org/elf/gabi4+/ch4.eheader.html \
https://refspecs.linuxbase.org/elf/gabi4+/ch5.pheader.html \
https://en.wikipedia.org/wiki/Page_%28computer_memory%29 \
https://stackoverflow.com/questions/4888067/how-to-get-linux-kernel-page-size-programmatically \
https://greek0.net/elf.html \
http://www.skyfree.org/linux/references/ELF_Format.pdf \
https://linuxhint.com/understanding_elf_file_format/

## Basic Structure

The ELF (Executable and Linking Format) file format is used for
object files, binaries, shared libraries and core dumps in Linux.

It has two views: a linking view and a execution view. The linking view
can be accessed by the Section Header Table and the execution view by the
Program Header Table.

The Program Header Table has an entry for each __Segment__, while the
Section Header Table has one for each __Section__.

Segments are basically just a group of 0 or more Sections with
information on how to make a process image.

![ELF Structure Image](https://upload.wikimedia.org/wikipedia/commons/thumb/7/77/Elf-layout--en.svg/541px-Elf-layout--en.svg.png)

As you can see in the image above, the file structure is divided in:

* __ELF Header__ - Always located at the beginning of the file

* __Program Header Table__ - Usually just after the ELF Header
(it may not exist)

* __Sections__ - Usually in the middle (it may not exist)

* __Section Header Table__ - Usually at the end (it not exist)

Obs .: Segments and Program Headers are synoyms, and Sections and
Sections Headers are sometimes treated like so too. However,
this can lead to confusion, so in this text we will refer to
Sections as a region in the middle of the file and Section
Headers as an entry in the Section Header Table.

![ELF Dissecated](https://upload.wikimedia.org/wikipedia/commons/thumb/e/e4/ELF_Executable_and_Linkable_Format_diagram_by_Ange_Albertini.png/440px-ELF_Executable_and_Linkable_Format_diagram_by_Ange_Albertini.png)

## ELF Header

The ELF Header occupy 52 bytes if it was compiled to 32 bits and 64 bytes
if it was compiled to 64 bits. The command `readelf` can be used with
the argument `-h` to read it easily. The following is a description for
each field in the header, for more detail and possible values of each one
check out the sources above (wikipedia and linux-audit contain a lot of
details).

* __0x00__ Magic (4 bytes) - This four bytes tell this is a ELF file. The
values are 0x7f (DEL) 0x45 (E) 0x4c (L) 0x46(F).

* __0x04__ Class (1 byte) - Determine the architecture for the file.

	* __1__ - 32 bit architecture
	* __2__ - 64 bit architecture

* __0x05__ Endianness (1 byte) - Determine the file's endianness, necessary
to read the file from the 0x10 offset.

	* __1__ - Little-Endian
	* __2__ - Big-Endian

* __0x06__ Version (1 byte) - The ELF version being used. The current and
original ELF version is 1, so the value here is always 1. The linux kernel
further specifies that a value of 0 here is invalid.

* __0x07__ OS/ABI (1 byte) - Identifies the target operating system ABI
(Application Binary Interface). It is often set to 0 (System V) regardless
of the target plataform, which means the ELF doesn't use any OS-specific
extension.

* __0x08__ ABI Version (1 byte) - in most cases specifies the ABI version
when needed, but its interpretation depends on the target ABI. For OS/ABI
value 3 (linux) this field specifies the ABI version of the dynamic linker.

* __0x09__ Padding (7 bytes) - Currently ununsed.

* __0x10__ Object File Type (2 bytes) - Determine the type of file,
indicating its purpose. The 4 most common values are:

	* __4__ - CORE
	* __3__ - DYN, shared object file
	* __2__ - EXEC, executable file
	* __1__ - REL, relocatable file, before linked into an executable
	file

* __0x12__ Instruction Set Architecture (2 bytes) - Tell the instruction
set the file was made for. Here are some of the most know:

	* __2__ - SPARC
	* __3__ - x86
	* __8__ - MIPS
	* __40__ - ARM
	* __62__ - amd64

* __0x14__ Version, again (4 byte) - Just like in offset __0x06__, this
tells the ELF version of the file.

* __0x18__ Entry Point (4 bytes if 32 bits, 8 bytes if 64) - Memory address
from which the process starts executing. If the file has no entry point
(if it is a relocatable file for example) the value is 0.

* __0x1c__/__0x20__ Program Header Table Location (4 or 8 bytes) - Points
to the start of the program header table, which usually follows the file
header immediately, making the offset for it 0x34 or 0x40 (depending
if it is 32 or 64 bits). If the file has no Program Header Table (if it
is a relocatable file for example) the value is 0.

* __0x20__/__0x28__ Section Header Table Location (4 or 8 bytes) - Points
to the start of the Section Header Table, which is usually the last part
of the file (if you take the file size and subtract by the product of the 
Section headers size and number you get it). If the file has no Section Header
Table the value is 0.

* __0x24__/__0x30__ Flags (4 bytes) - Interpretation of this field depends
the target architecture.

* __0x28__/__0x34__ ELF Header Size (2 bytes) - Size of the ELF Header.
Usually it is 64 bytes for 64 bit and 52 bytes for 32 bit.

* __0x24__/__0x36__ Program Header Table Entry Size (2 bytes) - Size of an
entry in the program header table. All entries have the same size.

* __0x2c__/__0x38__ Number of Entries in Program Header Table (2 bytes) -
It may be 0 if there if there is no Program Header Table.

* __0x2e__/__0x3a__ Section Header Table Entry Size (2 bytes) - Size of an
entry in the program header table. All entries have the same size.

* __0x30__/__0x3c__ Number of Entries in Section Header Table (2 bytes) -
It may be 0 if there if there is no Section Header Table.

* __0x32__/__0x3e__ Section Header Names Entry (2 bytes) - Index of the
Section header entry that contains the Section names. If there is no
Section name string table, this member holds the value of `SHN_UNDEF`

* __0x34__/__0x40__ First address in the file that is not from the ELF
Header. The ELF header size is either 0x34 (52 bytes) or 0x40 (64 bytes),
depending on the architecture.

## Program Headers (Segments)

Segments or Program Headers can located by the Program Header Table, which
is basically an array with entries for each Segment. They describe how to
create a process/memory image for runtime execution, making them only
meaningful for executables and shared object files when running the file.

Segments group single Sections by attribute (like permissions), making
loading the process image more efficient. The following is the layout of
a Segment entry in the Program Header Table, which can be better
visualized with the command `readelf -l`:

* __0x00__ Segment Type (4 bytes)

Value | Meaning
------|---------
__0x00000000__ (PT_NULL) | Segment is ununsed, usually the first Segment
__0x00000001__ (PT_LOAD) | Loadable Segment
__0x00000002__ (PT_DYNAMIC) | Dynamic linking information, Segment is holding the .dynamic Section
__0x00000003__ (PT_INTERP) | Interpreter information, Segment is holding the .interp Section
__0x00000004__ (PT_NOTE) | Auxiliary information
__0x00000005__ (PT_SHLIB) | Reserved
__0x00000006__ (PT_PHDR) | Segment containing program header table itself
__0x00000007__ (PT_TLS) | Thread-local storage template, common in statically linked binaries
__0x60000000__ to __0x7fffffff__ | range reserved for OS specific semantics


* __0x18__/__0x04__ Flags (4 bytes) - Segment flags, each bit represent
a different flag. Due to alignment reasons, in 64 bit architectures
this field is at offset 0x04 and in 32 bit architecures it is at 0x18.

	* Permission Flag Bits - If a bit is 0, the Segment don't
	have the permission.

	Value | Meaning
	--------------|---------
	0x1 (first bit) | Execute
	0x2 (second bit) | Write
	0x4 (third bit) | Read
	0x0ff00000 (bits 28-20) | reserved for operating system-specific semantics
	0xf0000000 (bits 32-28) | reserved for processor-specific semantics 

* __0x04__/__0x08__ Offset (4 bytes in 32 bits, 8 in 64) - position of
the Segment in the file.

* __0x08__/__0x10__ Virtual Memory Address
(4 bytes in 32 bits, 8 in 64) - Address in which the Segment should be
loaded in memory.

* __0x0c__/__0x18__ Segment Physical Address
(4 bytes in 32 bits, 8 in 64) - reserved to store the Segment's
physical address, when on systems in which this is relevant.

* __0x10__/__0x20__ Segment Size in File
(4 bytes in 32 bits, 8 in 64) - Size in bytes of the Segment in
the file. It can be 0.

* __0x14__/__0x28__ Segment Size in Memory
(4 bytes in 32 bits, 8 in 64) - Size in bytes of the Segment when
loaded to memory. It can be 0.

* __0x1c__/__0x30__ Segment Alignment
(4 bytes in 32 bits, 8 in 64) - Specify the alignment to be used
when Segment is loaded to memory. The value should be an positive
power of 2.

	* Note that the 32 bit offset
	here is __0x1c__ and not __0x18__ due to the __Flags__ field
	(second item in this list).

* __0x20__/__0x38__ From this point on these bytes don't belong
to the Segment.

#### Loading in memory

only Segments of type PT_LOAD will be loaded in memory. Every other
Segment is mapped within the memory range of a PT_LOAD Segment.

#### Page Size

A Page is a fixed-length block of virtual memory. It is the smallest unit
of data for memory management. Therefore, when enforcing
read/write/execute permissions, they are going to affect at least an
entire Page.

Segments have read/write/execute permissions associated with them, that may
differ from one Segment to another. Because of it, different Segments must
be loaded in different Pages.

Compilers guarantee that by making each Segment's virtual address a
multiple of the system's page size (which can be seen in linux with the
command `getconf PAGESIZE`).

This is why grouping Sections in Segments is efficient: We group the
Sections by permissions at compile time and just load the Segments
to different pages when running the executable.

If instead we had just the Sections we would have to do this grouping
when launching the executable or load each Section to a different page
to guarantee that the permissions would be respected.

## Sections and Section Headers

Sections hold instructions and data used for linking and relocation
(connecting symbolic references with symbolic definitions). They contain
all the information of the file, except the ELF Header, the Program Header
Table and the Section Header Table.

They are only needed at linktime in order to build a executable.

Every Section has only one entry in the Section Header Table but
Section Headers in the table may not have a Section.

This is the strcuture of each entry in the Section Header Table,
and can be seen with the command `readelf -S`:

* __0x00__ Section Name (4 bytes) - The value is a index into the Section
Header string table Section (`.shstrtab`), that has a null terminated
string representing the Section name.

* __0x04__ Section Header Type (4 bytes) - Identifies the type of this
header.

Value | Meaning
------|---------
__0x0__ (SHT_NULL) | Section header entry ununsed, don't have associated Section
__0x1__ (SHT_PROGBITS) | Program data
__0x2__ (SHT_SYMTAB) | Symbol table
__0x3__ (SHT_STRTAB) | String table
__0x4__ (SHT_RELA) | Relocation entries with addends. A file may have multiple relocation Sections
__0x5__ (SHT_HASH) | Symbol hash table
__0x6__ (SHT_DYNAMIC) | Dynamic linking information
__0x7__ (SHT_NOTE) | Notes
__0x8__ (SHT_NOBITS) | Program space with no data (.bss)
__0x9__ (SHT_REL) | Relocation entries, no addends. A file may have multiple relocation Sections
__0x0A__ (SHT_SHLIB) | Reserved
__0x0B__ (SHT_DYNSYM) | Dynamic linker symbol table
__0x0E__ (SHT_INIT_ARRAY) | Array of constructors
__0x0F__ (SHT_FINI_ARRAY) | Array of destructors
__0x10__ (SHT_PREINIT_ARRAY) | Array of pre-constructors
__0x11__ (SHT_GROUP) | Section group
__0x12__ (SHT_SYMTAB_SHNDX) | Extended Section indices
__0x13__ (SHT_NUM) | Number of defined types

* __0x08__ Flags (4 bytes for 32 bits and 8 bytes for 64) - 1-bit flags
that describe miscellaneous attributes.

Value | Meaning
------|---------
__0x1__ (first bit) | Writable
__0x2__ (second bit) | Occupies memory during execution
__0x4__ (third bit) | Executable
__0x10__ (fifth bit) | Might be merged
__0x20__ (sixth bit) | Contains null-terminated strings
__0x40__ (seventh bit) | 
__0x80__ (eighth bit) | Preserve order after combining
__0x100__ (ninth bit) | Non-Standard OS specific handling required
__0x200__ (tenth bit) | Section is member of a group
__0x400__ (eleventh bit) |  Section hold thread-local data

* __0x0c__/__0x10__ Virtual Address
(4 bytes for 32 bits and 8 bytes for 64) - Virtual address of the Section
in memory, used for Sections that will be loaded.

*  __0x10__/__0x18__ Section Offset
(4 bytes for 32 bits and 8 bytes for 64) - Position of the Section in the
file.

* __0x14__/__0x20__ Section Size
(4 bytes for 32 bits and 8 bytes for 64) - Size in bytes of the Section in
the file. It can be 0.

* __0x18__/__0x28__ Section Index (4 bytes) - Contain the index of the
entry in the Section Header Table. Its interpretation depends on the
Section type.

* __0x1c__/__0x2c__ Section Information (4 bytes) - Contains extra
information about the section. Its interpretation depends on the Section
type.

* __0x20__/__0x30__ Section Alignment
(4 bytes for 32 bits and 8 bytes for 64 bits) - Alignment required for the
Section. The value must be a power of 2. The values 0 and 1 mean there are
no alignment constraints.

* __0x24__/__0x38__ Section Entry Size
(4 bytes for 32 bits and 8 bytes for 64 bits) - Some Sections hold a table
of fixed-size entries (like a symbol table) and the value in this field
is the size of the entries. It is 0 if the Section don't hold a
fixed-size entries.

* __0x28__/__0x40__ From this point on these bytes don't belong to the
Section Header Table but since the Section Header Table is located at
the end of the file, there is probably nothing.
