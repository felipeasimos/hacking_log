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

class CacheTimingAttack {

	protected:

		struct impl;
		std::unique_ptr<impl> pimpl;

		void handle_lambda_calls(std::function<bool (unsigned int n_calls, void* addr)>, void* addr) const;

		std::tuple<int, size_t> open_executable(const char* executable) const;

		std::tuple<void*, size_t> mmap_file(int fd, size_t size) const;
	public:

		unsigned int hit_begin=0;
		unsigned int hit_end=0;

		bool was_accessed(void* addr) const;

		void wait_for_access(void* addr) const;

		//return false in the given function to stop execution,
		//return true to wait for another access
		void call_when_offset_is_accessed(const char* executable, unsigned int offset, std::function<bool (unsigned int n_calls, void* addr)>) const;

		unsigned int time() const;
		void flush(void* addr) const;

		//flush + probe_raw
		unsigned int probe(void* addr) const;

		unsigned int time_operation(void* addr) const;

		virtual void operation(void* addr) const;

		CacheTimingAttack();
		~CacheTimingAttack();

		CacheTimingAttack(const CacheTimingAttack&);

		CacheTimingAttack& operator=(const CacheTimingAttack& other);
};

#endif
