#ifndef VISUALIZER_HPP
#define VISUALIZER_HPP

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <ios>
#include <utility>

#include <stdio.h>

#define MIN_HILL_VALUE 0.000008

using hit_miss_map = std::map<unsigned int, std::pair<double, double>>;

class Visualizer {

	protected:

		struct impl;
		std::unique_ptr<impl> pimpl;

		unsigned int find_farthest_peak(hit_miss_map&);

		FILE* open_file(const char*);
		void csv_write_row(FILE*, std::vector<std::string>);
	public:
		void to_csv(hit_miss_map& map, const char* filename);

		Visualizer();
		~Visualizer();

		Visualizer(const Visualizer&);

		Visualizer& operator=(const Visualizer& other);
};

#endif
