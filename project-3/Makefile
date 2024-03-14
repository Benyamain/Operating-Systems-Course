compile:
	@gcc -Wall -c syscall-riscv64.S -o syscall-riscv64.o
	@gcc -Wall -c posix-calls.c -o posix-calls.o
	@gcc -Wall -c crt-callmain-riscv64.S -o crt-callmain-riscv64.o
	@gcc -Wall -c Llibc.c -o Llibc.o
	@ar rcs lib4490.a \
		syscall-riscv64.o \
            	posix-calls.o \
            	crt-callmain-riscv64.o \
            	Llibc.o
	@gcc -Wall -c shellprint.c -o shellprint.o
	@ld -TLlinker.ld -static -nostdlib shellprint.o -L. -l4490 -o shellprint
	@./shellprint
