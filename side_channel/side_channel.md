# Side Channel Attacks

dependencies:
	[Paging](../os/paging.md)

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
https://eprint.iacr.org/2020/907.pdf

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

Name | Observes | Footprint | Speed | Deduce |
-----|----------|-----------|-------|------|
[Flush+Reload](#flushreload) | Times it takes to access memory | Cache hits/misses rate, memory access | (Much slower than Flush+Flush) | If victim accessed memory
[Flush+Flush](#flushflush) | Execution time of `flush` instruction | Minimal number of cache hits | 496 KB/s | If victim accessed memory
Jump Over ASLR | 
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

In the assmebly:

* `address` is stored in `%ecx` ("`"c" (address)`").

* `time` is returned in the register `%eax` ("`"=a" (time)`");

* 4 bytes are read from the memory address in `%ecx`, `address` ("`movl (%1), %%eax`").

* `rdtsc` instruction reads the 64 bit counter, returning the low 32 bits of the counter
in `%eax` and the high 32 bits in `%edx` (which we ignore, since the number is small).

* copies counter to `%esi` ("`movl %%eax, %%esi`").

* after reading the memory, the counter is read again. The result is subtracted
from the previous counter value ("`subtl %%esi, %%eax`").

* `mfence` and `lfence` are used to serialize the instruction stream (force processor
to don't execute them in parallel). `lfence` ensures all load instructions preceding
it have completed and no instruction following it executes before it has executed.
`mfence` orders all memory accesses, fence instructions and the `clflush` instruction (
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
