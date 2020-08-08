#ifndef FLUSH_CALIBRATION_HPP
#define FLUSH_CALIBRATION_HPP

#include <memory>
#include <map>
#include <utility>

#include "../cache_timing_attack/cache_timing_attack.hpp"
#include "cache_info/cache_info.hpp"
#include "../visualizer/visualizer.hpp"

#define DEFAULT_NUM_SAMPLES 100000000

using hit_miss_map = std::map<unsigned int, std::pair<double, double>>;

class FlushCalibrator {

	protected:
		struct impl;
		std::unique_ptr<impl> pimpl;

		unsigned int hit(CacheTimingAttack& attack);
		unsigned int miss(CacheTimingAttack& attack);

		void hit_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples);
		void miss_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples);

		void normalize_histograms(hit_miss_map&, unsigned int num_samples);

		bool is_distance_enough(std::pair<double, double> hit_miss);

		unsigned int find_left_hit_boundry(hit_miss_map&, unsigned int peak_x);
		unsigned int find_right_hit_boundry(hit_miss_map&, unsigned int peak_x);

		unsigned int find_hit_peak(hit_miss_map&);

		void find_hit_range(CacheTimingAttack& attack, hit_miss_map&);
	public:

		//minimum chance of a probe being
		//classified as a true positive
		//according to collected data
		double sensibility = 1.0;

		hit_miss_map calibrate(
				CacheTimingAttack& attack,
				const char* filename=nullptr,
				double sensibility=1.0,
				unsigned int num_samples=DEFAULT_NUM_SAMPLES
				);

		FlushCalibrator();
		~FlushCalibrator();

		FlushCalibrator(const FlushCalibrator&);
		FlushCalibrator& operator=(const FlushCalibrator&);
};

#endif
