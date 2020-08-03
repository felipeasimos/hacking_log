# Hacking Log

This repository has some notes that care about the technical and/or
use details of tools/exploits used many different attacks. The concepts
needed to understand these subjects are also covered, to some extent.

## Headers

Each `.md` file has a `sources:` part that has all the links used to
gather information to write said file. There also may be a
`dependencies:` part with links to other files in this repository which
cover the required concepts to understand the file.

## Repository Structure
```
|-- README.md
|-- binary_exploitation
|   |-- assembly
|   |   |-- alignment.md
|   |   |-- basics.md
|   |   |-- endianness.md
|   |   |-- registers.md
|   |   `-- stack.md
|   |-- buffer_overflow
|   |   |-- buffer_overflow.md
|   |-- gdb
|   |   `-- gdb.md
|   |-- heap_exploitation
|   |   `-- heap.md
|   `-- rop
|       |-- first
|       |   `-- first.md
|       |-- fourth
|       |   `-- fourth.md
|       |-- second
|       |   `-- second.md
|       `-- third
|           `-- third.md
|-- elf
|   |-- compilation.md
|   `-- layout.md
|-- networking
|   |-- nmap
|   |   `-- nmap.md
|   `-- osi
|       `-- osi.md
|-- os
|   |-- caching.md
|   |-- compiler.md
|   |-- heap.md
|   |-- memory.md
|   |-- os.md
|   |-- paging.md
|   |-- permissions.md
|   `-- processes.md
`-- side_channel
    `-- side_channel.md
```
