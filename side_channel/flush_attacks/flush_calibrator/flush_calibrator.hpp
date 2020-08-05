#ifndef FLUSH_CALIBRATION_HPP
#define FLUSH_CALIBRATION_HPP

#include <memory>
#include <map>
#include <utility>

#include "../cache_timing_attack/cache_timing_attack.hpp"
#include "cache_info/cache_info.hpp"
#include "../visualizer/visualizer.hpp"

//this is value is used to determine when a "hill" is coming
//in the distribution graph. Since the histograms are normalized
//there is no problem for this to be hardcoded, but you can
//change to do experiment with the results.

//in my system, it doesn't make really a difference if this value
//is 0, but on systems where the histograms overlap a lot it
//would make a major difference (the algorithm may not even work)
#define FC_VALLEY_MIN 0.000025

using hit_miss_map = std::map<unsigned int, std::pair<double, double>>;

class FlushCalibrator {

	protected:
		struct impl;
		std::unique_ptr<impl> pimpl;

		void access_addr(void* addr);

		unsigned int hit(CacheTimingAttack&, void* addr);

		unsigned int miss(CacheTimingAttack&, void* addr);

		void hit_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples);
		void miss_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples);

		void normalize_histograms(hit_miss_map&, unsigned int num_samples);

		bool is_hit_faster(hit_miss_map&);

		double pair_idx(std::pair<double, double> hist_sample, unsigned int idx);

		std::pair<unsigned int, unsigned int> find_peak_and_valley(hit_miss_map&, CacheTimingAttack&);

		unsigned int _find_threshold(hit_miss_map&, unsigned int valley_x, unsigned int peak_x);

		unsigned int find_threshold(CacheTimingAttack& attack, hit_miss_map&);
	public:

		hit_miss_map calibrate(CacheTimingAttack& attack, unsigned int num_samples=1000000);

		void histogram(CacheTimingAttack& attack, const char* filename="histogram.csv", unsigned int num_samples=1000000);

		FlushCalibrator();
		~FlushCalibrator();

		FlushCalibrator(const FlushCalibrator&);
		FlushCalibrator& operator=(const FlushCalibrator&);
};

#endif
