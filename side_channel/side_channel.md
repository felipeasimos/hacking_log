# Side Channel Attacks

dependencies:
	[Paging](../os/paging.md)
	[Caching](../os/caching.md)

sources:
https://en.wikipedia.org/wiki/Side-channel_attack \
https://eshard.com/posts/ches2019_post/ \
https://tches.iacr.org/index.php/TCHES/article/view/7387 \
https://gruss.cc/files/flushflush.pdf \
https://defuse.ca/flush-reload-side-channel.htm \
https://www.felixcloutier.com/x86/clflush \
https://eprint.iacr.org/2013/448.pdf \
https://www.felixcloutier.com/x86/lfence \
https://github.com/IAIK/flush_flush \
https://github.com/nepoche/Flush-Reload \
https://eprint.iacr.org/2020/907.pdf \
http://www.cs.ucr.edu/~nael/pubs/micro16.pdf
https://people.eecs.berkeley.edu/~kubitron/courses/cs252-S11/lectures/lec09-prediction2.pdf \
https://en.wikipedia.org/wiki/Branch_target_predictor \
http://cseweb.ucsd.edu/~j2lau/cs141/week8.html \
http://euler.mat.uson.mx/~havillam/ca/CS323/0708.cs-323007.html \
https://digitalassets.lib.berkeley.edu/techreports/ucb/text/CSD-89-553.pdf \
https://www.youtube.com/watch?v=T8_Jvt2T6d0&list=PL1C2GgOjAF-IWC1AEXqWKFmAgZdQRJfZ6 \
https://en.wikipedia.org/wiki/Branch_(computer_science) \
https://en.wikipedia.org/wiki/Translation_lookaside_buffer \
https://download.vusec.net/papers/anc_ndss17.pdf \
https://github.com/vusec/revanc \
https://www.vusec.net/projects/anc/ \
http://palms.ee.princeton.edu/system/files/SP_vfinal.pdf \
https://www.youtube.com/watch?v=vpGI1ggKzC4

A side channel attack is any attack that is based on information
gained from the implementation of a computer system, rather than
weakness in the implemented algorithm itself.

Timing information, power consumption, eletromagnetic leaks or even
sound can provide an extra source of information, which can be
exploited.

The physical metadata left by operations can be used to gather
insight.

## Grouping

We can divide side channels in the type of data they use (sound,
timing, radio frequency, etc) or divide in two categories:

* __Non-profiled attacks__ - Attacker only can access a closed
target.

	* Differential Power Analysis (DPA)
	* Correlation Power Analysis (CPA)
	* Mutual Information Analysis (MIA)
	* Differential Deep Learning Analysis (DDLA)

* __Profiled attacks__ - Attacker has access to an open copy of
the target device and so can use it to identify patterns in the
leaked data to relevant information prior to the attack.

	* Template attacks
	* Support Vector Machine
	* Random Forests
	* Deep Learning

## Attacks

Below you can check the attacks in more detail, but
here there is a simple table of what kind of information
they observe, what they can deduce from it, the footprint
left behind and how much information per second it can gather:

Name | Observes | Footprint | Speed | Deduce | Paper Link |
-----|----------|-----------|-------|--------|------------|
[Flush+Reload](#flushreload) | Times it takes to access memory | Cache hits/misses rate, memory access | (Much slower than Flush+Flush) | If victim accessed memory | https://eprint.iacr.org/2013/448.pdf
[Flush+Flush](#flushflush) | Execution time of `flush` instruction | Minimal number of cache hits | 496 KB/s | If victim accessed memory | https://gruss.cc/files/flushflush.pdf
[Jump Over ASLR](#jump-over-aslr) | Times it takes to execute system call/code from another process | Depend on choosen targets | Depend on choosen targets | Address of a specific instruction from kernel or a running process | http://www.cs.ucr.edu/~nael/pubs/micro16.pdf
[Prime+Probe](#primeprobe) | Times it takes to access memory | Cache hits/misses rate, memory access | slower than Flush+Reload | If victim accessed cache line | http://palms.ee.princeton.edu/system/files/SP_vfinal.pdf (the blackhat video is pretty great too)
AnC | 
Keyboard Acoustic Emanations | Sound from key presses | None, if using same device as victim's, else it depends on a physical device nearby | Hardware dependent | What key where pressed

### Flush+Reload

[Sharing memory pages between non-trusting processes](../os/paging#page-sharing)
is a common method of reducing the memory footprint. This can be
exploited to monitor access to addresses in shared pages.

This attack targets the last level cache, so the attacker's
program don't need to share a core with the victim.

It can identify access to specific addresses, giving
a high resolution and minimizing false positives.

#### Key Concepts

* To maintain isolation between non-trusting processes, the
system relies on hardware mechanisms to enforce
read only or __copy-on-write__ semantincs for these pages.
__copy-on-write__ pages need to copy the entire page
when something is written to it (hence the name), which
cause a delay. We can use this to know if a page was
shared.

* When a program accesses a shared page in memory, the contents
of it are cached.

* By using the `clflush` instruction, we can flush a _line_
from all caches levels.

* Retrieving memory from cache levels
closer to memory takes longer than retrieving from cache levels
closer to the core.

#### Vulnerability

The time it takes to access the memory can tell us if it has
been accessed.

#### Attack

The attack relies on sharing pages with between spy and victim
processes.

1. With shared pages, the spy can ensure that a specific
address is evicted from the whole cache hierarchy (`clflush`).

At this point, the spy know the address is no longer in any cache.

2. Spy waits for some time.

3. Spy reloads the address, measuring the time it takes to load it.

	* If during step 2 the victim has accessed the address, the
	line will be in the cache, and the reload will take a short time.

	* If during step 2 the victim hasn't accessed the address, the
	reloading will take significantly longer.

NOTE: If the victim access the line while the reloading is happening, the
access won't be detected.

```
int probe(char *address) {

	volatile unsigned long time;

	asm __volatile__ (
	
		"mfence \n"
		"lfence \n"
		"rdtsc \n"
		"lfence \n"
		"movl %%eax, %%esi \n"
		"movl (%1), %%eax \n"
		"lfence \n"
		"rdtsc \n"
		"subl %%esi, %%eax \n"
		"clflush 0(%1) \n"
		: "=a" (time)
		: "c" (address)
		: "%esi", "%edi"
	);

	return time < threshold;
}
```

In the assembly:

* `address` is stored in `%ecx` ("`"c" (address)`").

* `time` is returned in the register `%eax` ("`"=a" (time)`");

* 4 bytes are read from the memory address in `%ecx`, `address` ("`movl (%1), %%eax`").

* `rdtsc` instruction reads the 64 bit counter, returning the low 32 bits of the counter
in `%eax` and the high 32 bits in `%edx` (which we ignore, since the number is small).

* copies counter to `%esi` ("`movl %%eax, %%esi`").

* after reading the memory, the counter is read again. The result is subtracted
from the previous counter value ("`subtl %%esi, %%eax`").

* `mfence` and `lfence` are used to serialize the instruction stream (force processor
to don't execute them in parallel).

* `lfence` ensures all load instructions preceding
it have completed and no instruction following it executes before it has executed.

* `mfence` orders all memory accesses, fence instructions and the `clflush` instruction (
It is not ordered with respect to other functions and thus, it is not enough to ensure
ordering).

To achieve sharing, the spy `mmaps` the victim's executable file into the spy's virtual
address space (by opening the file and using the file descriptor as an argument to
`mmap`).

Since the LLC is physically tagged, entries in the cache depend on the physical address
of the shared page without relying on the virtual addresses the page is mapped to.
Consequently, our virtual addresses will affect the victim without the need to worry
about diversification techniques like ASLR.

#### Downfalls

* Several processor optimizations may result in false positives due
to speculative memory acceses.

#### Optimizations

* One way to improve resolution without affecting error rate is to
target memory accesses that occur frequently, such as a loop body. The
attack won't be able to discern between separate accesses, but the
likelihood of missing all the iterations of the loop is small.

#### Setup

The detection relies on a threshold value, that determines if a cache
miss of hit occured. Prior to the attack, we need to find an optimal
value for the threshold, since its value is highly system dependent.

The asm code using for the attack can also be used for measuring
and finding the best threshold.

We can force misses by calling `clflush` before timing the access
and also get hit by just accessing repeatedly.

### Flush+Flush

A fast and stealth cache attack.

CPU caches leak significant information. As an attempt to prevent this,
proposed detection mechanisms assume that all cache attacks cause more
cache hits and cache misses than benign applications and use hardware
performance counters for detection.

Well, not Flush+Flush! It only relies on the execution time of the
`clflush` instruction, which depends on wheter data is cached or not.

#### Key Concepts

Same as [Flush+Reload](#flushreload) and also:

* `clflush` can abort early in case of a cache miss (takes a short
time). In case of hit, it has to trigger eviction on all
caches (takes some time).

#### Vulnerability

The attack depends on the `clflush` instruction (clear cache line)
execution time. It will take longer to execute if the data is cached.

The time it takes for `clflush` to execute tells us if the address
was in a cache or not, which tells us if a victim process has accessed
it.

#### Attack

The attack has only one phase, that is executed in a loop:

1. Execute `clflush` and track its execution time.

	* If it takes some time, address wasn't in the cache
	(victim didn't accessed it).

	* If it doesn't take much time, address was in cache
	(victim accessed it).

Mind that `clflush` do two things: give us info about the
victim access and also prepares the cache hierarchy for
the next iteration of the loop (by taking the address
out of the caches).

So, despite the name begin Flush+Flush, we actually
just do one flush per iteration.

```
int probe(char *address) {

	volatile unsigned long time;

	asm __volatile__ (
	
		"mfence \n"
		"lfence \n"
		"rdtsc \n"
		"lfence \n"
		"movl %%eax, %%esi \n"
		"clflush 0(%1) \n"
		"lfence \n"
		"rdtsc \n"
		"subl %%esi, %%eax \n"
		: "=a" (time)
		: "c" (address)
		: "%esi", "%edi"
	);

	return time < threshold;
}
```

In the assmebly:

* `address` is stored in `%ecx` ("`"c" (address)`").

* `time` is returned in the register `%eax` ("`"=a" (time)`");

* `rdtsc` instruction reads the 64 bit counter, returning the low 32 bits of the counter
in `%eax` and the high 32 bits in `%edx` (which we ignore, since the number is small).

* copies counter to `%esi` ("`movl %%eax, %%esi`").

* after flushing the memory, the counter is read again. The result is subtracted
from the previous counter value ("`subtl %%esi, %%eax`").

* `mfence` and `lfence` are used to serialize the instruction stream (force processor
to don't execute them in parallel). `lfence` ensures all load instructions preceding
it have completed and no instruction following it executes before it has executed.
`mfence` orders all memory accesses, fence instructions and the `clflush` instruction (
It is not ordered with respect to other functions and thus, it is not enough to ensure
ordering).

You can also check a better code than this one in the [research github
repository](https://github.com/IAIK/flush_flush).

Just like in [Flush+Relod](#flushreload), we can `mmap` the victim executable
file to perform the attack (completely ignoring ASLR).

#### Optimizations

* If the address is mapped to another core cache, a penalty of
3 cyles is added to the minimum execution time for cache hits.

#### Setup

Similar to the [Flush+Reload](#flushreload) setup. Just time how
long misses and hit take (by forcing them to happen with `clflush`)
to find the optimal threshold.

### Prime+Probe

Find cache set the victim is using.

#### Key Concepts

* When data is not in the cache a __cache miss__ occur.

* When a __cache miss__ occur, it takes some time. More time
than it would take in a __cache hit__ (if the data was already
in the cache).

#### Vulnerability

* Since __cache misses__ take longer than __cache hits__, we can
infer when a cache line is not in the cache.

* LLC is inclusive with the caches closer to the cache in most
architectures.

#### Attack

1. __Prime__:

	1. The attacker chooses a cache-sized memory buffer.

	2. Attacker access all the lines in the buffer, filling
	the cache with its data (controlling the state of the buffer).

2. When the victim executes, it will overwrite some
cache lines.

3. __Probe__: The attacker can now access the entire buffer again and
see what lines take longer to access.

Lines that take longer are the ones the victim is using, and thus
causing a cache miss.

#### Downfalls

* Can take some time.

#### Optimizations

* Avoid Probing all the cache (too much stuff!) as it will take
some time. Probing a single set at a time.

* LLC is divided in _slices_. Each core has its _slice_ and its
access to its slice is faster than to other cores slices. We can
find out which line belongs to which core:

	1. Start with a buffer with size 0.

	2. Iteratively add lines to the pool as long as there is
	no self-eviction.

	3. Self-eviction is detected by priming a potential new
	member accessing the current pool and timing other access
	to the potential new member.

	4. Repeat until you have a buffer that covers the full
	cache set.

### Jump Over ASLR

Uses the BTB (Branch Target Buffer) to recover all
random bits of a kernel address and reduce entropy
of user-level randomization.

#### Key Concepts

* Usually, when we have a branch instructions (that
makes us execute a different sequence of
instructions) in our code, we do speculative execution,
because in these cases we are able to run instructions
before knowing which one we should.

* We say a branch is _taken_ when we jump to another
sequence of instructions, and _not taken_ if we just
keep executing the same sequence.

* That means we start executing instructions that are not
guaranteed to be the ones we actually need (in which case
we undo this execution). 

* As you can imagine, executing the wrong instructions has
a performance penalty, so there are mechanisms for better
predicting the right instructions to run.

* __Branch target buffer (BTB)__ is a hashtable, indexed by
the current program counter, which contain branch target
addresses, and maybe also the target instruction.

* __BTBs__ are useful because with them we can know the
possible next instructions addresses at the first stage
of our pipeline.

* Remember, the pipeline stages are, in order:

Abbreviation | Name |
-------------|------|
IF | Instruction fetch |
ID | Instruction decode |
EX | Execute |
MEM | Memory access |
WB | Register write back |

* With __BTBs__ the processing sequence is like so:

```
		IF (Query BTB)
		       |
	Was there an entry for current Program Counter (PC)?
	   /                                               \
	 YES                                               NO
	  |                                                 |
    ID (Use predicted PC from BTB, process the branch)   ID (Fetch and process next instruction normally)
    	    |                                                                     |
      Was the branch taken?                                              Was the branch taken?
      /                   \                                    		 /                   \
     YES                  NO                                            YES                  NO
      |                    |                                             |                     \
EX (Continue as normal)   EX (Flush mispredicted             EX (add instruction and            EX (Continue as normal)
			  instructions, delete entry         target address to BTB.
			  from BTB and restart at correct    Flush mispredicted instructions.
			  instruction)                       Restart at the correct instruction)
 
```

* __Cross Domain Collisions (CDC)__ - Are collisions in the BTB between user-level
and kernel-level branches. It occurs when the two branches, located at distinct
virtual addresses, map to the same entry in BTB with the same target addresses. This
can happen since BTB in recent processors ignore upper-order bits of the addresses.

* __Same Domain Collisions (SDC)__ - Are BTB Collisions, just like __CDC__, the difference
being that it happens between two user-level branches from two different processes.

* __Kernel ASLR (KASLR)__ - is the ASLR procedure for
kernel modules. Kernel virtual addresses can easily
be translated to physical by subtrating a predefined
`PAGE_OFFSET` (0xffffffff80000000 in 64 bit) from it.

	* Without it, the kernel image is always placed in
	the same physical address.

	* With it, a sequence of random bits is generated during
	boot. They are used to calculate the randomized offset
	at which the kernel is placed in physical memory.

* This is the layout of a kernel address in 64 bits:

```
47                 30 29          21 20                 0
.--------------------.--------------.-------------------.
|    Always Fixed    |  Randomized  | Determined during |
| 111111111111111110 | during  load |    Compilation    |
`--------------------'--------------'-------------------´
```

#### Vulnerability

The vulnerability is something inherit in hastables: collisions. If two processes
end up using the same entry in the BTB, we can detect when a branching instruction
happened in another application, and in which address, by doing time measurements.

#### Attack

We need to first get a sample of the victim code. Let's say we found the following:

```
3000 <victim_func>:
3000 push %rbp
3001 mov %rsp,%rbp
3004 nop
3005 nop
.  .  .
3021 jmp <T2>
3023 nop
.  .  .
302d <T1>
302d nop
.  .  .
3037 <T2>
3037 nop
.  .  .
3041 pop %rbp
3042 retq
```

So we would code a `spy_func` accordingly:

```
3000 <spy_func>:
3000 push %rbp
3001 mov %rsp,%rbp
3004 rdtscp
3005 nop
.  .  .
3021 jmp <T2>
3023 nop
.  .  .
302d <T1>
302d nop
.  .  .
3037 <T2>
3037 nop
.  .  .
3041 rdtscp
.  .  .
306c pop %rbp
306d retq
```

The goal is to record the execution time of the
spy process when the measured `jmp` instruction
aligns with a similar instruction in the victim
process.

##### Recovering KASLR Addresses Bits

Due to the way translation of virtual to physical
kernel addresses happen, we can easily derandomize
the static kernel image once we discover one specific
address.

To perform the attack we:

1. Find system call that has a branch instruction and
a small number of overall instructions (to minimize noise).

2. Create list of all possible addresses for the branch
instruction, taking into account the randomization scheme
and offset of that instruction in kernel code.

3. For each address **A** in the list, perform these
steps:

	1. Allocate buffer at required address in spy process

	2. Load buffer with a block of code containing a single
	jump instruction. The loading is done in a way that creates
	collisions in the BTB with a possible kernel branch instruction
	at address A. This block of code should also contain an instruction
	to measure the time to execute the jump.

	3. The target branch instruction in the kernel is activated by
	executing the identified system call.

	4. The block of code in the spy process is executed a number of times
	and the number of cycles taken to execute it is recorded.

4. Analyze the results. The block with higher average cycle measurement
corresponds to the situation when the jump instruction in the spy's code
block collides with the kernel branch at address **A**.

##### Recovering User Process Address Bits

Much like the attack for KASLR, the attack
used for ASLR measure the time it takes for a branch
instruction to execute.

To perform the attack we:

1. Analyze victim executable file to find
functions that can be triggered and also allocate
jump instructions in such functions.

2. The spy process performs the following steps for
each possible address **A** where the victim branch
instruction can be:

	1. Allocate buffer at required address.

	2. Fills buffer with code containing a single
	jump instruction at address **A** and code to
	measure time.

	3. Trigger the desired function in victim
	process in order to force victim to create a
	BTB entry.

	4. Waits for activity to complete.

	5. It executes the jump block several times
	and measures the execution time.

3. Finally, the spy discovers the address, just like
in the KASLR attack.

#### Downfalls

* The attack to find out User process addresses doesn't
work when the spy and victim process don't reside in the
same core.

#### Optimizations

To ensure that the victim and spy processes execute in
the same core (_co-residency_), we can:

* Inject dummy processes in all other virtual cores
in order to alter the schduling algorithm.

* Execute spy on all virtual cores.

In linux, processes can use `sched_setaffinity()` to schedule
itself to a specific core, which can be useful in both methods
above.

### AnC

Just through memory accesses, we can infer which cache sets
have been accessed after a targeted MMU __Page Table__ walk.

Knowing the cache sets we can identify the offsets of the
target __Page Table__ entries at each __Page Table__ level,
hence derandomizing ASLR.

#### Key Concepts

* For every virtual address the processor access, the MMU
translates it to a physical address.

* Each core has a __Translation Lookaside Buffer (TLB)__
that stores the recent translations, to speed up memory access.

* When a __TLB miss__ occur, the MMU needs to walk the __Page
Tables Tree (PTT)__ of the process to perform the translation.

* To improve performance in __TLB misses__, __PTs__ are stored in the
fast data caches just like the process data, and therefore also in the
LLC.

* When the MMU do a __PTT__ walk, it reads __PTEs__ for
each layer in the __PTT__ and stores each one in different
cache lines in __L1D__ (first layer cache, which stores only data), if
they are not there already (We know they are in different lines because
each __PT__ is a page, and thus each entry will be in a different page).

* Since ASLR is used as a security mechanism, the __PTs__ are now sensitive
information, which is kept in the same storage as our program's data!

* Usually in a core microarchitecture there are three levels of CPU caches:

	1. __L1D__ (data) and __L1I__ (instructions) are closest to
	the cache.

	2.  __L2__ is unified for both data and instructions. __L1__ and
	__L2__ caches are private to each core. Data present in __L1__ is
	not necessarily present in __L2__.

	3. __L3__ is shared amongt every core. All data in __L1__ and
	__L2__ are also in __L3__.

* With the key concepts above in mind, we need to answer 3 questions
to pull off this exploit:

	1. Which cache lines are loaded from memory during the __PT__ walk?

		* Since the __Last Level Cache (LLC)__ is inclusive to
		__L1D__ (LLc is usually __L3__), if we evict a cache line
		from __L3__, it would also evict it from __L1D__.

		* Every loaded __PTE__ will be in __LLC__ and __L1D__.

		* We can monitor cache sets at the __LLC__ and detect
		MMU activity due to a __PT__ walk at the affected cache
		sets (Prime+Probe). It is enough to identify the offset
		of the __PTE__ cache lines within a page.

	2. Which page offsets do these cache lines belong to?

		* Given 2 different physical memory pages, if their first
		cache lines belong to the same cache set, then their
		other cache lines share cache sets as well.

		* Given an identified cache set with __PT__ activity, we
		can determine the offset of the cache line that hosts the
		__PTE__.

	3. What are the offsets of the target __PTEs__ in these cache
	lines?

		* Access pages that are `x` bytes apart from our target
		virtual address `v`. If we do that for different values,
		we can see a change in one of the cache sets that show
		MMU activity.

		* By monitoring these cache sets we can see a "moving"
		cache set (the new cache set affected directly follow
		the previous one).

#### Attack

There is some actions we need to be able to perform to pull this off:

1. Trigger __PT__ walks:

2. Prime+Probe and the MMU signal

3. Cache color

4. Evict+Time attack on the MMU

5. Sliding PT Entries

6. ASLR Solver

7. Evicting Page Table Caches

8. Dealing With Noise
