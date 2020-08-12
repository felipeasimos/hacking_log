#ifndef FLUSH_FLUSH_HPP
#define FLUSH_FLUSH_HPP

#include "../cache_timing_attack/cache_timing_attack.hpp"

class FlushFlush : public CacheTimingAttack {

	public:
		unsigned int time_hit(void* addr) const;
		unsigned int time_miss(void* addr) const;
		unsigned int probe(void* addr) const;

		FlushFlush(const char* exec, unsigned int offset);

		FlushFlush(const FlushFlush&);
		FlushFlush(FlushFlush&&);

		FlushFlush& operator=(const FlushFlush&);
		FlushFlush& operator=(FlushFlush&&);
};

#endif
