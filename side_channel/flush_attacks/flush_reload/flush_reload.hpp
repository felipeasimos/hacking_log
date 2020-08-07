#ifndef FLUSH_RELOAD_HPP
#define FLUSH_RELOAD_HPP

#include "../cache_timing_attack/cache_timing_attack.hpp"

class FlushReload : public CacheTimingAttack {

	public:
		void operation() const;

		FlushReload(const char* exec, unsigned int offset);

		FlushReload(const FlushReload&);
};

#endif
