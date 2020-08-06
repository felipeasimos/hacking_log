#ifndef FLUSH_FLUSH_HPP
#define FLUSH_FLUSH_HPP

#include "../cache_timing_attack/cache_timing_attack.hpp"

class FlushFlush : public CacheTimingAttack {

	public:
		void operation(void* addr) const;

		FlushFlush();

		FlushFlush(const FlushFlush&);
};

#endif
