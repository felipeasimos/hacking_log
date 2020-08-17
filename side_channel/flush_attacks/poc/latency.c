#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

#define CTA_ASM_RDTSC_OP(instr) "mfence\n"\
				"lfence\n"\
				"rdtsc\n"\
				"lfence\n"\
				"movl %%eax, %%esi\n"\
				instr "\n"\
				"mfence\n"\
				"lfence\n"\
				"rdtsc\n"\
				"subl %%esi, %%eax\n"	

#define FLUSH_FLUSH_STR "clflush 0(%1)"

#define FLUSH_RELOAD_STR "movl (%1), %%eax"

#define MIN(x,y) x < y ? x : y

#define NUM_SAMPLES 100000

void flush_addr(void* addr) {

	asm volatile(
			"mfence\n"
			"lfence\n"
			"clflush 0(%0)\n"
			"lfence\n"
			:
			: "c"(addr)
			: "rax");
}

void access_addr(void* addr) {

	asm volatile(
			"mfence\n"
			"lfence\n"
			"movl (%0), %%eax\n"
			"lfence\n"
			:
			: "r"(addr)
			: "eax"
		);
}

unsigned int ff(void* addr){

	volatile unsigned int time=0;

	asm volatile (
			CTA_ASM_RDTSC_OP(FLUSH_FLUSH_STR)

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

unsigned int fr(void* addr){
	
	volatile unsigned int time=0;
	asm volatile (
			CTA_ASM_RDTSC_OP(FLUSH_RELOAD_STR)
			"clflush 0(%1)\n"

			: "=a"(time)
			: "c"(addr)
			: "%esi", "%edi"
		);

	return time;
}

unsigned int get_avg_frequency(){

	FILE* file = fopen("/proc/cpuinfo", "r");

	char buf[512]={0};
	unsigned int frequency=0;;
	unsigned int tmp=0;


	for( unsigned int counter=0; fgets(buf, 512, file) && counter < 4; ){

		if( !strncmp("cpu MHz", buf, 7) ){
			sscanf(buf, "cpu MHz\t: %u.", &tmp);

			frequency+=tmp;
			counter++;
		}
	}

	fclose(file);

	return frequency/4;
}

int main(int argc, char** argv){

	if( argc < 3 ) printf("2 arguments must be given\n");

	unsigned int (*choosen_func)(void* addr)=ff;

	int fd = open(argv[1], O_RDONLY);
	size_t map_size = lseek(fd, 0L, SEEK_END);

	if( !map_size || fd == -1 ){

		fprintf(stderr, "Couldn't open file!\n");
		exit(1);
	}

	printf("binary opened!\n");

	unsigned int page_size = sysconf(_SC_PAGE_SIZE);

	if( map_size & page_size ){

		map_size |= page_size;
		map_size++;
	}

	void* base_address = mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd, 0);

	printf("binary allocated!\n");

	unsigned int offset;
	if(!sscanf(argv[2], "%x", &offset)){
	
		fprintf(stderr, "Couldn't parse offset!\n");
		exit(1);
	}

	printf("offset parsed! 0x%x\n", offset);

	void* addr = (char*)base_address + offset;

	printf("address to attack: %p\n", addr);
	printf("base address: %p\n", base_address);

	unsigned int** hit_frequency = calloc(NUM_SAMPLES, sizeof(unsigned int*));

	unsigned int time = 0;
	for(unsigned int i =0; i < NUM_SAMPLES; i++){

		hit_frequency[i] = calloc(2, sizeof(unsigned int));

		access_addr(addr);
		time = choosen_func(addr);
		hit_frequency[i][0] = time;
		
		flush_addr(addr);
		time = choosen_func(addr);
		hit_frequency[i][1] = time;
	}

	printf("data gathering done! Writing to file...\n");

	FILE* csv=fopen("hit.csv", "w");
	fprintf(csv, "timestamp, hit, miss\n");

	for(unsigned int i=0; i < NUM_SAMPLES; i++){

		fprintf(csv, "%u, %u, %u\n", i, hit_frequency[i][0], hit_frequency[i][1]);
	}

	fclose(csv);

	close(fd);
	munmap(base_address, map_size);

	free(hit_frequency);

	return 0;
}
