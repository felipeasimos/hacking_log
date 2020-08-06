#include "flush_calibrator.hpp"

using FC = FlushCalibrator;

struct FC::impl{

	CacheInfo cache = CacheInfo();
	Visualizer visualizer;
	char* test_memory;
	char* test_memory_base_address;
};

void FC::access_addr(void* addr){

	asm volatile ("movq (%0), %%rax\n"
			:
			: "r" (addr)
			: "rax");
}

unsigned int FC::hit(CacheTimingAttack& attack, void* addr){

	access_addr(addr);
	return attack.time_operation(addr);
}

unsigned int FC::miss(CacheTimingAttack& attack, void* addr){

	attack.flush(addr);
	return attack.time_operation(addr);
}

void FC::hit_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples){

	for(unsigned int i=0; i < num_samples; i++)
		map[hit(attack, pimpl->test_memory)].first++;
}

void FC::miss_loop(CacheTimingAttack& attack, hit_miss_map& map, unsigned int num_samples){

	for(unsigned int i=0; i < num_samples; i++)
		map[miss(attack, pimpl->test_memory)].second++;
}

void FC::normalize_histograms(hit_miss_map& histograms, unsigned int num_samples){

	for(auto& [ x, sample ] : histograms){

		sample.first /= num_samples;
		sample.second /= num_samples;
	}
}

bool FC::is_hit_faster(hit_miss_map& histograms){

	std::pair<unsigned int, double> hit_max(0,0.0);
	std::pair<unsigned int, double> miss_max(0,0.0);

	for( auto const& [x, hist] : histograms ){

		if( hist.first > hit_max.second ) hit_max = std::make_pair(x, hist.first);

		if( hist.second > miss_max.second ) miss_max = std::make_pair(x, hist.second);
	}

	//hit is faster if its peak comes first
	return hit_max.first < miss_max.first;
}

double FC::pair_idx(std::pair<double, double> hist_sample, unsigned int idx){

	if(idx) return hist_sample.second;
	else return hist_sample.first;
}

std::pair<unsigned int, unsigned int> FC::find_peak_and_valley(hit_miss_map& histograms, CacheTimingAttack& attack){

	std::pair<unsigned int, double> peak_max(0,0.0);
	std::pair<unsigned int, double> valley_min(0,0.0);

	unsigned int peak_i = !attack.hit_is_faster;
	unsigned int valley_i = attack.hit_is_faster;

	for( auto const& [ x, sample ] : histograms ){

		//peak
		if( pair_idx(sample, peak_i) > peak_max.second )
			peak_max = std::make_pair(x, pair_idx(sample, peak_i));

		//valley (get the first value greater than FC_VALLEY_MIN)
		if( pair_idx(sample, valley_i) > FC_VALLEY_MIN && !valley_min.second )
			valley_min = std::make_pair(x, pair_idx(sample, valley_i));
	}

	return std::make_pair(valley_min.first, peak_max.first);  
}

unsigned int FC::_find_threshold(hit_miss_map& map, unsigned int valley_x, unsigned int peak_x){

	std::pair<unsigned int, double> threshold(0,0.0);

	for(unsigned int x=peak_x; x < valley_x; x++){

		std::pair<double, double> sample = map[x];

		if( threshold.second < sample.first + sample.second )
			threshold = std::make_pair( x, sample.first + sample.second );
	}

	return threshold.first;
}

unsigned int FC::find_threshold(CacheTimingAttack& attack, hit_miss_map& histograms){

	attack.hit_is_faster = is_hit_faster(histograms);

	auto [valley_x, peak_x] = find_peak_and_valley(histograms, attack);

	attack.threshold = _find_threshold(histograms, valley_x, peak_x);

	return attack.threshold;
}

hit_miss_map FC::calibrate(CacheTimingAttack& attack, unsigned int num_samples){

	//time: { hits, misses }
	hit_miss_map histograms;

	hit_loop(attack, histograms, num_samples/2);
	miss_loop(attack, histograms, num_samples/2);
	normalize_histograms(histograms, num_samples);

	find_threshold(attack, histograms);

	return histograms;
}

void FC::histogram(CacheTimingAttack& attack, const char* filename, unsigned int num_samples){

	hit_miss_map map = calibrate(attack, num_samples);

	pimpl->visualizer.to_csv(map, filename);
}

FC::FlushCalibrator() : pimpl(std::make_unique<FC::impl>()){

	//fill an entire set
	unsigned int size = pimpl->cache.block_size() *
				pimpl->cache.num_ways();

	pimpl->test_memory_base_address = new char[ size ];
	pimpl->test_memory = pimpl->test_memory_base_address + size/2;
}

FC::~FlushCalibrator()=default;

FC::FlushCalibrator(const FC& other) :
	pimpl(std::make_unique<FC::impl>(*other.pimpl)){}

FC& FC::operator=(const FC& other){

	*pimpl = *other.pimpl;
	return *this;
}
