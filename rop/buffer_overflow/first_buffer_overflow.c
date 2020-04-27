#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){

	char password[16];
	int passcheck=0;

	printf("Type in the password: ");

	scanf("%s", (char*)&password);

	//verification
	if( strcmp(password, "password1") ){
	
		printf("\nACCESS DENIED\n");

	} else {

		printf("\nACCESS ALLOWED\n");
		passcheck=1;
	}

	if( passcheck ){

		system("/bin/sh");
	}

	return 0;
}
