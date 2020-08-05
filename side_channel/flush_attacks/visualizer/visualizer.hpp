#ifndef CACHE_TIMING_ATTACK_HPP
#define CACHE_TIMING_ATTACK_HPP

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <ios>

#include <stdio.h>

using hit_miss_map = std::map<unsigned int, std::pair<double, double>>;

class Visualizer {

	protected:

		struct impl;
		std::unique_ptr<impl> pimpl;

		FILE* open_file(const char*);
		void csv_write_row(FILE*, std::vector<std::string>);
	public:
		void to_csv(const char* filename);

		Visualizer(hit_miss_map& map);
		~Visualizer();

		Visualizer(const Visualizer&);

		Visualizer& operator=(const Visualizer& other);
};

#endif
