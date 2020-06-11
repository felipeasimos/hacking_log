#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void impossible_shell(){

	system("/bin/sh");
}

int password_is_correct(char* passwd){

	char buffer[16];
	strcpy(buffer, passwd);

	return !strncmp(buffer, "passwd1", 16);
}

int main(int argc, char* argv[]){

	if( password_is_correct(argv[1]) ){

		printf("PASSWORD IS CORRECT!\n");
		printf("i don't feel like giving you a shell though😝\n");

	} else {

		printf("PASSWORD IS INCORRECT!\n");
		printf("try harder next time\n");
	}
}
