# Operating System

sources:
http://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf \
http://www.nastooh.com/teaching/William_Stallings_Operating_Systems_7th_Edition.pdf

A computing system has many parts:

* __hardware__ - eletronic circuits (processor, memory, input/output ports, etc)
and eletric, optic or mechanic periferals (keyboard, mouse, disks, etc).

* __software__ - programs used by the user (text editors, web browser, games, etc).

* __Operating System__ (OS) - lies between __hardware__ and __software__.

## Objectives

At the end of the day, the OS is just a software, but a special one, since
it affects every other aspect of the system. Like any other software, it
was made with some objectives in mind.

The operating system is a huge: it handles stuff from the _drivers_
(responsible for dealing with this diversity of devices) level to
the utilities programs and the graphical interface.

Lets see the main objectives of an OS.

### Device Abstraction

There is a huge gap between eletronic circuits and the software
programs. There are many types of hardware devices, which the
use depends on the technology used for each one.

This diversity can be troublesome for a software developer. That is
where the first problem to be solved is:

__Abstract device access to applications__

The OS tries to accomplish this with 3 points in mind:

* Provide a simpler access interface to applications than the one that
would be used if dealing with the device directly.

* Make applications independent from hardware.

* Define access interfaces that work the same way for whatever specific
technology is being used by the device.

### Resource Management

The OS is responsible for defining policys for resource management to
resolve eventual conflicts that may happen.

For example, two applications trying to access a device at the same
time.

## Features

To accomplish the objectives stated above. Each system resource
has specific requirements to be managed and abstracted properly.

With this in mind, the main functionalities of an OS are:

* __Process management__ - distribute the processing
capacity in a fair manner between applications,
avoiding that an application consume all the resources while
respecting priorities defined by the user.

* __Memory management__ - give each application a memory region
just for it, isolated from other processes and the OS itself. This
improves the stability and safety of the system as a whole (avoid
errors or malicious programs of interefering with other processes).

Also, abstractions done by this allow the OS to increase the RAM
memory (using secondary memory) if it isn't enough for the
running processes. This is done in a transparent way, without
the applications knowing.

* __Device management__ - each device has it own unique
traits, but the OS can create a single abstraction to be
used for all devices of each type
(memory, network card, keyboard, etc). This is mostly
done by _drivers_.

* __File management__ - create directories and files,
defining their access interface and rules on its use.

* __Protection management__ - define resources that
each user can use and how (write, read, execute, etc):

	* define user groups.

	*  identify users connected to the system
	(through authentication).
	
	* define and apply rules for controlling
	access to resources.

	* register the use of resources by the users.

These are the basic features, but modern systems also
handle the graphical interface, networking, power sources,
etc.

The features are usually inter-dependent: one depend on
another.

## Policy and Management

These are two important concepts that may look very similar:

* __Policy__ - abstract decision system. High level.

Ex: high level algorithm to choose how much memory an
application should receive

* __Mechanism__ - procedures used to implement policys. Low level.

Ex: give or take memory from applications.

Another way to look at it is their relationship with each other.

__mechanisms__ must be generic enough to allow changes in policys
without the need to also be changed.

## Elements Of An Operating System

* __Kernel__ - resource management and main abstractions used
by applications and utlity programs. Execute in _privileged
mode_.

* __Boot code__ - hardware initialization require many complex
tasks. Also loads the __Nucleus__ to memory and initialize its
execution.

* __Drivers__ - Modules with specific code to access physical
devices.

* __Utility Programs__ - programs that facilitate the use of the
OS, offering additional features to the kernel like graphical
interface, file formatting, device configuration, etc.

```
	.--------------.------------------.
	| Applications | utility programs |   User
  ----  |--------------'------------------| --------
	| Kernel (has all the main OS     |  System
	|  features inside it)            |
	|------------------.--------------|
	|    Boot code     |   Drivers    | software
  ----  |------------------'--------------| --------
	|       device controllers        | hardware
	|---------------------------------|
	|       physical devices          |
	`---------------------------------´

```

## Architecture

Nowadays, most computers have the following parts:

* __Processor (CPU)__- Most computers nowadays have more than
one processor. It is responsible for reading instructions
and data, processing it and sending the results back to
memory or another periferal. In its most basic form, it
is made out of:

	* __Arithmetic and Logic Unit (ALU)__ - do
	calculations and logical operations.

	* __Registers__ - some are used for many operations
	and some have special purposes (program counter,
	stack pointer, status flags, etc).

Modern processors may also have:

	* __Cores__ - processing units. A single processor
	may have multiple, and a single core may have
	many internal logical processors (_hyperthreading_)

	* __Cache__ - each processor may have a cache just
	for it.

* __Memory Management Unit (MMU)__ - can be built-in inside
the processor chip. Every processor has it. It intercepts the
the access of the processor to the address and control buses.
It is responsible for:

	* analysing addresses accessed by the processor

	* validate addresses accessed by the processor

	* perform addressing convertions if necessary

	* execute write/read in an address (given a request
	by the processor)

	* It may also have a _cache_ for optimizing performance

![MMU Scheme](https://external-content.duckduckgo.com/iu/?u=https%3A%2F%2Ftse1.mm.bing.net%2Fth%3Fid%3DOIP.e-f2hw7oMhl1ObZPpmRZoAHaEK%26pid%3DApi&f=1)

* __Primary Memory__ - Stores data and programs. This memory is
typically volatile. Usually there are more than one, and they
are used as _caches_, making up the _cache hierarchy_.

* __I/O Modules__ - Move data between computer and the external
environment, which consists of a variety of devices, including
the __Secondary Memory__ (non-volatile, usually biggest memory
in the system, last level in the memory hierarchy).

* __System Bus__ - Provides communication among processors,
main memory and I/O Modules. There are three parts to it:

	* __Address Bus__ - indicates which position in memory
	to access.

	* __Control Bus__ - indicates the operation that will
	be done (write or read).

	* __Data Bus__ - transport the data to be sent between
	the processor and the memory or I/O module.

![System Bus Scheme](https://upload.wikimedia.org/wikipedia/commons/thumb/6/68/Computer_system_bus.svg/1200px-Computer_system_bus.svg.png)

## Interruptions and Exceptions

Both can stop the current execution flow in the processor.

### Interruptions

The processor and devices communicate through input/output
ports that can be write/read by the processor.

The processor is the one that starts the talk, when it needs
to do so, but sometimes the device may need to inform the
processor of something quickly. There is two options:

* Wait for the processor to talk to the device again
(can take some time, it may be busy!)

* notify the processor, by sending an _Interruption ReQuest_
(IRQ) through the __Control Bus__.

Upon receiveing the _IRQ_, the processor circuits suspend the
current execution and go to a predefined address, where an
__interruption handler__ code is. After its execution the
process return to the previous execution.

Each _IRQ_ comes with a value, so it is possible to know
why it has been send.

Interruptions are actually really common
(thousands of interruptions per second). They are used for
telling the processor when an event happens, for example,
a click in a mouse. Because of that the __interruption handler__
code must be small and execute fast.

Since each IRQ can use its own routine for treating the
interruption, most architectures have a
__Interruption Vector Table (IVT)__. Each entry points to
a interruption handling routine.

### Exceptions

Some events generated by the processor itself can redirect
the execution flow using the same mechanism as the interruptions.

We are talking about __exceptions__. Actions like illegal instructions,
divisions by zero and other software errors launch exceptions in the
in the processor, activating a routine responsible for handling exceptions.

## Protection

### Privilege Level

Kernel, drivers, utilities and applications are all made of code, but
their capacity to interact with hardware must be different.

Kernel and drivers can freely access hardware, to manage and configure it,
but applications and utilities must have a more restricted access, so they
don't interfere with the proper management and configuration being done. If
this wasn't imposed, malicious programs could do anything they wanted!

To differentiate them, modern processors have _execution privilege levels_.
This levels are controlled by special flags in the CPU, and are adjusted to
the code being executed.

In its most simple form we have:

* __Kernel Level (supervisor, system, monitor)__ - to code executed at this
level, all system features are at its disposal. All instructions and registers
can be used and all system resources can be accessed.

* __User Level (userspace)__ - to code executed at this level, only part
of the processor instructions and registers are available. An Exception
will be thrown if trying to execute prohibited instructions. Illegal
instructions are often dangerous, like the ones to directly access
an input/output port or reset the processor.

The flags responsible for this differentiation can only be modified by
__kernel level__ code.

### MMU

The MMU (Memory Management Unit) that we showed before also has an important
role in securing the system. It can create exclusive areas for
each process, isolated from the kernel and other processes. Applications
can't work around this, since this configuration can only be changed
by __kernel level__ code.

## System Calls

With the MMU isolating each process in its exclusive memory region, how
can we invoke routines offered by the kernel to access OS services and
hardware (using the abstractions the OS provides)?

Kernel code that can do all these stuff also lies in its own memory
region, isolated from all other processes.

We can't directly execute it, but we can use the interruption
mechanism to invoke these functionalities from the kernel.

Processors have specific instructions to invoke services from
the kernel, much like interruptions. When executed, these
instructions change the processor privilege to __kernel level__
and execute code in a predefined routine, just like an
__interruption handler__. In fact, so similar that this
mechanism is called __software interruption (trap)__.

The activation of a kernel routine using this interruption is
called __system call (syscall)__. OS define __syscalls__ to all
operations related to access of low level resources (files, periferals,
memory allocation, etc) or logical abstractions (creating and terminating
processes, synchronization, etc).

Usually some registers must be set before the __syscall__, so the processor
can get the necessary arguments and the _opcode_ for the it (so it knows
what specific __syscall__ is being called).

OS define hundreds of __syscalls__. The set of __syscalls__ of a system
define the _API_ (Application Programming Interface) of the OS.

__syscalls__ can be divided in:

* process management - create, load code, terminate, wait...

* memory management - allocation/deallocation/modify...

* file management - create, remove, write, read, close...

* communication - send/receive data...

* device management - read/write, change/read configs...

* system management - time, date, shutdown, reboot...
