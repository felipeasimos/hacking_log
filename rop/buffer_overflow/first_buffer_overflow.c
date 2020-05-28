#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){

	int passcheck=0;
	char password[16];

	//get user input
	printf("Type in the password: ");
	scanf("%s", (char*)&password);

	//check if it is correct
	if( strcmp(password, "password1") ){
	
		printf("\nACCESS DENIED\n");

	} else {

		printf("\nACCESS ALLOWED\n");
		passcheck=1;
	}

	//get shell
	if( passcheck ){

		system("/bin/sh");
	}

	return 0;
}
