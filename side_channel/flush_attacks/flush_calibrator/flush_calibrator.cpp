#include "flush_calibrator.hpp"

using FC = FlushCalibrator;

struct FC::impl{

	CacheInfo cache = CacheInfo();
	Visualizer visualizer;
};

unsigned int FC::hit(CacheTimingAttack& attack){

	attack.flush();
	attack.access();
	return attack.time_operation();
}

unsigned int FC::miss(CacheTimingAttack& attack){

	attack.flush();
	return attack.time_operation();
}


void FC::hit_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples){

	for(unsigned int i=0; i < num_samples; i++)
		map[hit(attack)].first++;
}

void FC::miss_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples){

	for(unsigned int i=0; i < num_samples; i++)
		map[miss(attack)].second++;
}

void FC::normalize_histograms(hit_miss_map& histograms, unsigned int num_samples){

	for(auto& [ x, sample ] : histograms){

		sample.first /= num_samples;
		sample.second /= num_samples;
	}
}

bool FC::is_distance_enough(std::pair<double, double> hit_miss){

	return ( !hit_miss.second ) ||
		( hit_miss.first/
		 (hit_miss.first+hit_miss.second) ) >= sensibility;
}

unsigned int FC::find_left_hit_boundry(hit_miss_map& hist, unsigned int peak_x){

	unsigned int i = peak_x;
	for(; i > 0; i--){

		if( !is_distance_enough(hist[i]) )
			return i + (peak_x!=i);
	}

	return i;
}

unsigned int FC::find_right_hit_boundry(hit_miss_map& hist, unsigned int peak_x){

	unsigned int i=peak_x;
	for(; i < hist.rbegin()->first; i++)

		if( !is_distance_enough(hist[i]) )
			return i - (peak_x!=i);

	return i;
}

unsigned int FC::find_hit_peak(hit_miss_map& hist){

	std::pair<unsigned int, double> peak(0, 0.0);

	for(auto const& [ x, sample ] : hist)

		if( peak.second < sample.first )
			peak = std::make_pair(x, sample.first);

	return peak.first;
}

void FC::find_hit_range(CacheTimingAttack& attack, hit_miss_map& histograms){

	unsigned int hit_peak = find_hit_peak(histograms);

	attack.hit_begin = find_left_hit_boundry(histograms, hit_peak);
	attack.hit_end = find_right_hit_boundry(histograms, hit_peak);
}

hit_miss_map FC::calibrate(
		CacheTimingAttack& attack,
		const char* filename,
		double sensibility,
		unsigned int num_samples) {

	//time: { hits, misses }
	hit_miss_map histograms;

	this->sensibility = sensibility;

	hit_loop(attack, histograms, num_samples/2);
	miss_loop(attack, histograms, num_samples/2);
	normalize_histograms(histograms, num_samples);

	find_hit_range(attack, histograms);

	if( filename ) pimpl->visualizer.to_csv(histograms, filename);

	return histograms;
}

FC::FlushCalibrator() : pimpl(std::make_unique<FC::impl>()){

}

FC::~FlushCalibrator()=default;

FC::FlushCalibrator(const FC& other) :
	pimpl(std::make_unique<FC::impl>(*other.pimpl)){}

FC& FC::operator=(const FC& other){

	*pimpl = *other.pimpl;
	return *this;
}
