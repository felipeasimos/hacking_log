# Paging

sources:
https://eprint.iacr.org/2013/448.pdf

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
