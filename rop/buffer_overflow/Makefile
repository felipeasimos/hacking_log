CC=gcc

OPTIONS=-fno-stack-protector -m32

CFILES:=$(wildcard *.c)

.PHONY: test clean
test: $(CFILES)
	for cfile in $(CFILES); do \
		$(CC) $(OPTIONS) $$cfile -o $${cfile%.*}; \
	done
clean:
	rm $(basename $(CFILES))
