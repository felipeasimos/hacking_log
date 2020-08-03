#ifndef CACHE_INFO_HPP
#define CACHE_INFO_HPP

#include <memory>
#include <stdio.h>
#include <ios>

class CacheInfo {

	private:

		struct impl;
		std::unique_ptr<impl> pimpl;

		void handle_file(FILE* f, std::function<void (FILE* f)> handler);
		void get_cache_info();
		void get_block_size();
		void get_num_sets();
		void get_num_ways();
	public:
		unsigned int block_size() const noexcept;
		unsigned int num_sets() const noexcept;
		unsigned int num_ways() const noexcept;

		//default constructor
		CacheInfo();
		~CacheInfo(); //in Pimpl, we need this to avoid error message

		//copy constructor
		CacheInfo(const CacheInfo&);

		//move constructor
		CacheInfo(CacheInfo&&) noexcept;

		//copy assignment
		CacheInfo& operator=(const CacheInfo& other);

		//move assignment
		CacheInfo& operator=(CacheInfo&& other) noexcept;
};

#endif
