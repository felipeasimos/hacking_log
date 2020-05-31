CC=gcc

.PHONY: first all clean
first:
	$(CC) rip.c -m32 -no-pie -fno-stack-protector -o rip
second:
	$(CC) rip.c -no-pie -fno-stack-protector -o rip
third:
	$(CC) rip.c -fno-stack-protector -o rip
fourth:
	$(CC) rip.c -o rip
clean:
	rm rip
