#ifndef CACHE_TIMING_ATTACK_HPP
#define CACHE_TIMING_ATTACK_HPP

#include <memory>

class CacheTimingAttack {

	protected:

		struct impl;
		std::unique_ptr<impl> pimpl;

	public:

		bool hit_is_faster = true;
		unsigned int threshold=0;

		bool was_accessed(void* addr);

		unsigned int time() const;
		void flush(void* addr) const;

		//flush + probe_raw
		unsigned int probe(void* addr);

		unsigned int time_operation(void* addr);

		virtual void operation(void* addr);

		CacheTimingAttack();
		~CacheTimingAttack();

		CacheTimingAttack(const CacheTimingAttack&);

		CacheTimingAttack& operator=(const CacheTimingAttack& other);
};

#endif
