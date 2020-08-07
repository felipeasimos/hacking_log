#include "flush_reload.hpp"

using FR=FlushReload;

void FR::operation() const {

	access();
	/*
	asm volatile ("movq (%0),%%rax\n"
			: 
			: "r" (addr)
			: "rax"
		);*/
}

FR::FlushReload(const char* exec, unsigned int offset) :
	CacheTimingAttack(exec, offset){};

FR::FlushReload(const FR& other) : CacheTimingAttack(other){};
