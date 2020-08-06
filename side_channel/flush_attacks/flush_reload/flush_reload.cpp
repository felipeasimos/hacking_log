#include "flush_reload.hpp"

using FR=FlushReload;

void FR::operation(void* addr) const {

	asm volatile ("movq (%0),%%rax\n"
			: 
			: "r" (addr)
			: "rax");
}

FR::FlushReload() : CacheTimingAttack(){};

FR::FlushReload(const FR& other) : CacheTimingAttack(other){};
