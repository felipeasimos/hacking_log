# Processes

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
       /-----> S -------\
       |       ^        |
       |       |        |
       V       V        V
D <--> R <---> T -----> Z 
       |                ^ 
       |                |
       \----------------/
```

## A Story about Adopted Zombie Children

Whenever a process ends execution, it exits and notify the parent process
about it. The parent process is supposed to execute the `wait` system call
which the dead child status and remove child from memory when completed.

If the Parent don't call `wait`, the dead child is kept in memory! In this
case when the parent process exits, the `init` process will adopt the
child. `init` periodically reaps (clear memory) its zombie children.
