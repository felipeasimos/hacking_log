CC=gcc

.PHONY: first all clean
first:
	$(CC) rip.c -m32 -no-pie -fno-stack-protector -o rip
second:
	$(CC) rip.c -fno-stack-protector -o rip
third:
	$(CC) rip.c -fno-stack-protector -o rip
fourth:
	$(CC) rip.c -o rip
disable_aslr:
	echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
enable_aslr:
	echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
clean:
	rm rip
