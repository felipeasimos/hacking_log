# Processes

sources:
https://gabrieletolomei.files.wordpress.com
http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf

Every program in linux run inside a process. And since everything in linux
is a file, the processes are too!

## PID

Each process has a `PID`, an id that identify them uniquely among other
processes. Here are some terminal commands you can use to find `PID` of a
program:

* `ps` - show currently running processes

* `pgrep` - return the `PID` (process ID) of the process which the name
has a substring that match the one you give

## Signals

In linux, processes can communicate with each other through signals: A 
process send a signal to another and the receiving process has a callback
function ready to be called when the signal is detected. This callback
can be set by the user but if they aren't the default one's will be used.

For example, when you are running a program in your terminal and press
`Ctrl`+`C`, you are actually sending a `SIGINT` signal to the program.
In the terminal you can also use `Ctrl`+`\` to send a `SIGQUIT` (abort
process immediatly, not as graceful as `SIGINT`) and `Ctrl`+`z` to send a
`SIGSTP` to pause the process. To send other signals you can use the `kill`
command or do it porgrammatically with a language like C/C++ (linux manuals
cover how to do this, check out `man`)

## TID

If you ever want to send/receive signals in a multithreaded program, you
may need to do it for a specific thread only. That's where the `TID` comes
in. It is like the `PID`, but for threads, since it identify a thread
uniquely among the other threads in a process.

## Types

There are two types of processes in linux: `background` and `foreground`.
If the user can interact with the program directly it is a `foreground`
one, otherwise it is a `background` one.

Daemons are background processes startedon system startup,
controlled via the `init` process.

## States

Processes in Linux can have one of the following processes:

* `R` - running or runnable (on run queue)

* `D` - uninterruptible sleep (it is sleeping and cannot be brought
back until I/O or event happens)

* `S` - sleeping (not running, waiting for event or signal)

* `T` - traced or stopped (stopped by signals, like `SIGINT` or `SIGSTOP`)

* `Z` - zombie (terminated, but still has entry in process table due to
parent not destroying it properly)

* `I` - Idle (just like `D` but don't contribute for statistics)

With the command `ps aux` we can see each process state in the `STAT`
column. Different symbols can be seen in the command, but most are
clearly explained [here (ask ubuntu)](https://askubuntu.com/questions/360252/what-do-the-stat-column-values-in-ps-mean)
and the `I` state (which is a new addition to the kernel) is better
explained [here (stack exchange)](https://unix.stackexchange.com/questions/462098/unrecognized-process-state-output-in-ps-command/462102#462102)
and [here (quora)](https://www.quora.com/What-does-Linux-process-state-I-mean-in-the-top-output#)

This is how processes change from one state to another:
           
```
       .-----> S -------.
       |       ^        |
       |       |        |
       v       v        v
D <--> R <---> T -----> Z 
       |                ^ 
       |                |
       `----------------´
```

## A Story about Adopted Zombie Children

Whenever a process ends execution, it exits and notify the parent process
about it. The parent process is supposed to execute the `wait` system call
which the dead child status and remove child from memory when completed.

If the Parent don't call `wait`, the dead child is kept in memory! In this
case when the parent process exits, the `init` process will adopt the
child. `init` periodically reaps (clear memory) its zombie children.

## Memory

When we execute a program, we are creating another process, which loads
the process image from the executable to a memory space given by the OS.

### Sections

The stuff loaded from the excutable to memory is divided in sections (we
can see this in more detail in the [elf](../elf/layout.md) section)
which is then divided in sections:

* __TEXT__ - has the code to be executed by the process, generated during
compilation and linking to libraries. This area has fixed size, calculated
during compilation and normally can only be accessed for reading and
executing.

* __DATA__ - contain the initialized static variables, which the value is
given at the source code of the program. These values are kept in the
executable file and loaded to this section when the process starts. global
and local static variables are kept here.

* __BSS__ - historically called _Block Started by Symbol_, this section
stores the non-initialized static variables. This section is separated from
__DATA__ because the initialized static variables need to have their value
stored in the executable file, and the separation allow the execution file
to be smaller.

* __HEAP__ - area used to store dynamic data, using operators like
`malloc` and `free`. This area has variable size, since it can
grow/shrink due to allocations/deallocations. The end of this section is 
defined by a pointer called _Program Break_, or simply _break_, that
can be tuned through system calls to grow/shrink the heap size.

* __STACK__ - area where the execution stack of the process is. In
multithreaded programs, this area only contain the stack of the main
thread. Since threads can be created and destroyed dynamically,
each thread store its execution stack in the heap or in another
memory region dedicated to this. The execution stack grows downwards,
we can see this in more deatil in the [assembly](../binary_exploitation/assembly/stack.md)
section.

You may noticed that only __TEXT__ has read and execute permission, while
the other sections usually have only read and write permissions (sometimes
we need to executed code from heap or make some section read only for more
security).

### Memory Layout

These sections are layed out in memory like so:

![Process Layout in Memory](https://gabrieletolomei.files.wordpress.com/2013/10/program_in_memory2.png?)

The addresses where the sections are usually randomized, just like the
space between them, due to ASLR (which we cover in the
[rop](../binary_exploitation/rop/third/third.md) section).
