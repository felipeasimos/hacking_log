#include "visualizer.hpp"

struct Visualizer::impl{

};

unsigned int Visualizer::find_farthest_peak(hit_miss_map& histograms){

	std::pair<unsigned int, double> peak1(0,0.0), peak2(0,0.0);

	for(auto const& [ x, sample ] : histograms){

		if( peak1.second < sample.first )
			peak1 = std::make_pair( x, sample.first );

		if( peak2.second < sample.second )
			peak2 = std::make_pair( x, sample.second );
	}

	return std::max(peak1.first, peak2.first);
}

FILE* Visualizer::open_file(const char* filename){

	FILE* file=fopen(filename, "w");

	if( !file ) throw std::ios_base::failure("Couldn't open file in Visualizer");

	return file;
}
void Visualizer::csv_write_row(FILE* file, std::vector<std::string> values){

	for(auto const& value : values)
		fprintf(file, "%s, ", value.c_str());

	//turn ", " in newline(\r\n)
	fseek(file, -2L, SEEK_CUR);
	fprintf(file, "\r\n");
}

void Visualizer::to_csv(hit_miss_map& map, const char* filename){

	FILE* file = open_file(filename);

	//headers
	csv_write_row(
			file,
			std::vector<std::string>{
				"timestamp",
				"address in cache",
				"address not in cache"
			}
		);

	unsigned int farthest_peak_x = find_farthest_peak(map);

	for( auto const& [ x, hit_miss ] : map ){

		csv_write_row(
				file,
				std::vector{
					std::to_string(x),
					std::to_string(hit_miss.first),
					std::to_string(hit_miss.second)
				}
			);

		if( x > farthest_peak_x &&
				hit_miss.first <= MIN_HILL_VALUE &&
				hit_miss.second <= MIN_HILL_VALUE ) break;
	}

	fclose(file);
}

Visualizer::Visualizer()
	: pimpl(std::make_unique<Visualizer::impl>()){}

Visualizer::~Visualizer()=default;

Visualizer::Visualizer(const Visualizer& other) :
	pimpl(std::make_unique<Visualizer::impl>(*other.pimpl)){}

Visualizer& Visualizer::operator=(const Visualizer& other) {

	*pimpl = *other.pimpl;
	return *this;
}
