#include "flush_flush.hpp"

using FF=FlushFlush;

void FF::operation(void* addr){

	asm volatile ("clflush (%0)\n"
			:
			: "r" (addr)
			:
		);
}

FF::FlushFlush() : CacheTimingAttack(){};

FF::FlushFlush(const FF& other) : CacheTimingAttack(other){};
