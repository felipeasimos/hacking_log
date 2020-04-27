# Buffer Overflow

In programming, a buffer is just a space in memory meant to store some data.
To do a Buffer Overflow is to write more data to a buffer than it was
designed to hold. By doing so, we write to memory locations we weren't
supposed to and thus alter the program expected execution.

Buffer Overflow vulnerabilities are accidentally created by developers
when using a low-level programming language, like C/C++ or assembly.
High-level programming languages like Javascript or Python check if the
data being write don't exceed the buffer size and the developers using them
usually don't need to worry about a possbile Buffer Overflow. However,
checking that makes execution slower when compared to programs made with a
low-level programming language. The trade-off between security and speed
must be taken into account when choosing which language to use.

## A Simple Demo

We will use the following program, that can be used in the
`first_buffer_overflow.c`:

```
#include <stdio.h>
#include <string.h>

int main(){

	char password[16];
	int passcheck=0;

	printf("Type in the password: ");

	scanf("%s", &password);

	//verification
	if( strcmp(password, "password1") ){
	
		print("\nACCESS DENIED\n");

	} else {

		print("\nACCESS ALLOWED\n");
		passcheck=1;
	}

	if( passcheck ){

		system("/bin/sh");
	}

	return 0;
}
```

The program writes the input to a buffer and compare it to a constant
string "password1". The variable passcheck changes value and is
ultimately the one responsible for allowing us to get a shell.

If we input `AAAAAAAAAAAAAAA` (15 A's) we get `ACCESS DENIED`
