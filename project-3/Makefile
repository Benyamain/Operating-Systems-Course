# Basic functions ("library") in assembly source:
ASRCS := syscall-riscv64.S crt-callmain-riscv64.S
# Basic functions ("library") in C source:
BSRCS := posix-calls.c Llibc.c
# Executable user programs (in C):
ESRCS := cat.c sleep.c xecho.c
CSRCS := $(BSRCS) $(ESRCS)

SOBJS := $(ASRCS:%.S=%.o)
BOBJS := $(BSRCS:%.c=%.o)
EOBJS := $(ESRCS:%.c=%.o)
LOBJS := $(SOBJS) $(BOBJS)
COBJS := $(BOBJS) $(EOBJS)
OBJS := $(COBJS) $(SOBJS)
EXECS := $(ESRCS:%.c=%)
LIB := lib4490.a

CFLAGS := -Wall -c -march=rv64imafd_zicsr_zifencei -mabi=lp64d -ffreestanding -nostdlib
# ASFAGS := -march=rv64imafd_zicsr_zifencei -mabi=lp64d
LDFLAGS := -TLlinker.ld -static -nostdlib

.SUFFIXES:

all: $(EXECS)

lib: $(LIB)

## Static pattern rules for replacing builtin/implicit rule:
$(EXECS) : % : %.o $(LIB)
	ld $(LDFLAGS) -o $@ $^
$(LIB) : $(LOBJS)
	ar rcs $(LIB) $(LOBJS)
$(COBJS) : %.o : %.c
	gcc $(CFLAGS) -o $@ $<
$(SOBJS) : %.o : %.S
	gcc $(CFLAGS) -o $@ $<
# See Secs 4.9, 4.10, Chapter 10 (esp 10.5) of GNU Make manual for Make v3.77

.PHONY: clean objclean
clean:
	rm -f $(EXECS) $(OBJS) $(LIB)
objclean:
	rm -f $(OBJS)

