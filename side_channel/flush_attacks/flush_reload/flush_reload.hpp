#ifndef FLUSH_RELOAD_HPP
#define FLUSH_RELOAD_HPP

#include "../cache_timing_attack/cache_timing_attack.hpp"

class FlushReload : public CacheTimingAttack {

	public:
		unsigned int time_hit(void* addr) const;
		unsigned int time_miss(void* addr) const;
		unsigned int probe(void* addr) const;

		FlushReload(const char* exec, unsigned int offset);

		FlushReload(const FlushReload&);
		FlushReload(FlushReload&&);

		FlushReload& operator=(const FlushReload&);
		FlushReload& operator=(FlushReload&&);
};

#endif
