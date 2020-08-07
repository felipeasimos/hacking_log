#ifndef CACHE_TIMING_ATTACK_HPP
#define CACHE_TIMING_ATTACK_HPP

#include <memory>
#include <stdio.h>
#include <ios>
#include <functional>
#include <tuple>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define CTA_TEST_EXEC "/usr/lib/x86_64-linux-gnu/gnome-calculator/libcalculator.so"
#define CTA_TEST_OFFSET 0x27550

class CacheTimingAttack {

	protected:

		struct impl;
		std::unique_ptr<impl> pimpl;

		virtual void operation() const;

		std::tuple<int, size_t> open_executable(const char* executable) const;

		std::tuple<void*, size_t> mmap_file(int fd, size_t size) const;	
	public:

		unsigned int hit_begin=0;
		unsigned int hit_end=0;

		unsigned int time() const;

		void flush() const;

		void access() const;

		unsigned int probe() const;

		unsigned int time_operation() const;

		bool was_accessed() const;

		bool was_accessed(unsigned int timestamp) const;

		unsigned int wait_for_access() const;

		//return false in the given function to stop execution,
		//return true to wait for another access
		void call_when_offset_is_accessed(std::function<bool (unsigned int time, void* addr)>) const;

		CacheTimingAttack(const char* executable, unsigned int offset);
		~CacheTimingAttack();

		CacheTimingAttack(const CacheTimingAttack&);

		CacheTimingAttack& operator=(const CacheTimingAttack& other);
};

#endif
