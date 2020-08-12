#include "flush_calibrator.hpp"

using FC = FlushCalibrator;

struct FC::impl{

	CacheInfo cache = CacheInfo();
	Visualizer visualizer;
};

void FC::hit_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples){

	void* addr = attack.addr();

	for(unsigned int i=0; i < num_samples; i++)
		map[attack.time_hit(addr)].first++;
}

void FC::miss_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples){

	void* addr = attack.addr();

	for(unsigned int i=0; i < num_samples; i++)
		map[attack.time_miss(addr)].second++;
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

double FC::peak_heuristic(std::pair<double, double> hit_miss){

	return hit_miss.first/(hit_miss.first+hit_miss.second);
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

	for(auto const& [ x, sample ] : hist){

		if( peak.second < peak_heuristic(sample))
			peak = std::make_pair(x, peak_heuristic(sample));
	}

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

std::pair<double, double> FC::benchmark(
		CacheTimingAttack& attack,
		unsigned int num_samples){

	double true_hit=0;
	double true_miss=0;

	unsigned int time=0;
	void* addr = attack.addr();

	for(unsigned int i=0; i < num_samples/2; i++){

		time = attack.time_hit(addr);
		if( attack.was_accessed(time) ) true_hit++;
	}

	for(unsigned int i=0; i < num_samples/2; i++){

		time = attack.time_miss(addr);
		if( attack.was_accessed(time) ) true_miss++;
	}

	return std::make_pair(2*true_hit/num_samples, 2*true_miss/num_samples);
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
