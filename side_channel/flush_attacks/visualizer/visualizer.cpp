#include "visualizer.hpp"

struct Visualizer::impl{

	std::string filename;
	hit_miss_map map;
};

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

void Visualizer::to_csv(const char* filename){

	FILE* file = open_file(pimpl->filename.c_str());

	//headers
	csv_write_row(
			file,
			std::vector<std::string>{
				"timestamp",
				"address in cache",
				"address not in cache"
			}
		);

	for( auto const& [ x, hit_miss ] : pimpl->map ){

		csv_write_row(
				file,
				std::vector{
					std::to_string(x),
					std::to_string(hit_miss.first),
					std::to_string(hit_miss.second)
				}
			);
	}

	fclose(file);
}

Visualizer::Visualizer(hit_miss_map& map) : pimpl(std::make_unique<Visualizer::impl>()){

	pimpl->map = map;
}

Visualizer::~Visualizer()=default;

Visualizer::Visualizer(const Visualizer& other) :
	pimpl(std::make_unique<Visualizer::impl>(*other.pimpl)){}

Visualizer& Visualizer::operator=(const Visualizer& other) {

	*pimpl = *other.pimpl;
	return *this;
}
