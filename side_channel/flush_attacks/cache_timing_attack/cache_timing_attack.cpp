#include "cache_timing_attack.hpp"

using CTA = CacheTimingAttack;

struct CTA::impl{

};

bool CTA::was_accessed(void* addr) {

	bool fast_execution = probe(addr) < threshold;

	return hit_is_faster ? fast_execution : !fast_execution;
}

unsigned int CTA::time() const {

	uint64_t a,d;

	asm volatile(
			"mfence \n"
			"rdtsc \n"
			"mfence \n"
			: "=a"(a), "=d"(d)
			: 
			:
		);

	//rdtsc is a x86 instruction. To write the 64 bit
	//timestamp, it writes the first MSR 32 bits in
	//EDX and the other 32 in EAX.

	return (d<<32) | a;
}

void CTA::flush(void* addr) const {

	asm volatile("clflush (%0)\n"
			:
			: "r"(addr)
			:
		);
}

unsigned int CTA::probe(void* addr){

	flush(addr);
	sched_yield(); //move this process to last in processr queue
	return time_operation(addr);
}

unsigned int CTA::time_operation(void* addr){

	unsigned int begin = time();
	operation(addr);
	return time() - begin;
}

void CTA::operation(void* addr){}

CTA::CacheTimingAttack() :
	pimpl(std::make_unique<CTA::impl>()){}

CTA::~CacheTimingAttack()=default;

CTA::CacheTimingAttack(const CTA& other) :
	pimpl(std::make_unique<CTA::impl>(*other.pimpl)){}

CTA& CTA::operator=(const CTA& other) {

	*pimpl = *other.pimpl;
	return *this;
}
