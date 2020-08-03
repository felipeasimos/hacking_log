#include "cache_info.hpp"

struct CacheInfo::impl{

	unsigned int block_size;
	unsigned int num_sets;
	unsigned int num_ways;
};

void CacheInfo::handle_file(FILE* f, std::function<void (FILE* f)> handler){

	if( f ) {
	
		handler(f);
		fclose(f);
	} else {
		throw std::ios_base::failure("Couldn't open file at /sys/devices/system/cpu/cpu0/cache/index3 directory");
	}
}

void CacheInfo::get_cache_info() {

	get_block_size();
	get_num_sets();
	get_num_ways();
}

void CacheInfo::get_block_size() {

	handle_file(
			fopen("/sys/devices/system/cpu/cpu0/cache/index3/coherency_line_size", "r"),
			[this](FILE* f){
				fscanf(f, "%u", &pimpl->block_size);
			}
		);
}

void CacheInfo::get_num_sets() {

	handle_file(
			fopen("/sys/devices/system/cpu/cpu0/cache/index3/number_of_sets", "r"),
			[this](FILE* f){
				fscanf(f, "%u", &pimpl->num_sets);
			}
		);
}

void CacheInfo::get_num_ways(){

	handle_file(
			fopen("/sys/devices/system/cpu/cpu0/cache/index3/ways_of_associativity", "r"),
			[this](FILE* f){
				fscanf(f, "%u", &pimpl->num_ways);
			}
		);
}

unsigned int CacheInfo::block_size() const noexcept {

	return pimpl->block_size;
}

unsigned int CacheInfo::num_sets() const noexcept {

	return pimpl->num_sets;
}

unsigned int CacheInfo::num_ways() const noexcept {

	return pimpl->num_ways;
}

//default constructor
CacheInfo::CacheInfo() : pimpl(std::make_unique<CacheInfo::impl>()){

	get_cache_info();
}

CacheInfo::~CacheInfo()=default;

//copy constructor
CacheInfo::CacheInfo(const CacheInfo& other) :
	pimpl(std::make_unique<struct CacheInfo::impl>(*other.pimpl)){};

//move constructor
CacheInfo::CacheInfo(CacheInfo&& other) noexcept=default;

//copy assignment
CacheInfo& CacheInfo::operator=(const CacheInfo& other){

	*pimpl = *other.pimpl;

	return *this;
}

//move assignment
CacheInfo& CacheInfo::operator=(CacheInfo&& other) noexcept=default;
