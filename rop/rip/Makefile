CC=gcc

.PHONY: first all clean
first:
	$(CC) rip.c -m32 -no-pie -fno-stack-protector -o rip
second:
	$(CC) rip.c -no-pie -fno-stack-protector -o rip
third:
	$(CC) rip.c -fno-stack-protector -o rip
clean:
	rm rip
