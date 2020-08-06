#include <stdio.h>

void inc(unsigned int a){

	a++;
}

void loop(unsigned int ah){

	unsigned int a=0;

	for(unsigned int i=0; i < ah; i++){

		for(unsigned int b=0; b < ah; b++){

			inc(a);
		}
	}

	printf("a: %u\n", a);
}

int main(int argc, char** argv){

	loop(20000);

	return 0;
}
