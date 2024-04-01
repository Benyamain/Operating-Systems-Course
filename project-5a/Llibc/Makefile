##################################################################
# About tabs/spaces in makefiles:
# A leading TAB means the line is an external command to be run by the shell!
# So:
# - Any external shell command (aka "recipe") line must begin with TAB
# - No other line in the makefile should begin with a TAB!
##################################################################

# Basic functions ("library") in assembly source:
ASRCS := syscall-riscv64.S crt-callmain-riscv64.S
# Basic functions ("library") in C source:
BSRCS := posix-calls.c Llibc.c
CSRCS := $(BSRCS)

SOBJS := $(ASRCS:%.S=%.o)
BOBJS := $(BSRCS:%.c=%.o)
LOBJS := $(SOBJS) $(BOBJS)
COBJS := $(BOBJS)
OBJS := $(COBJS) $(SOBJS)
LIB := lib4490.a

# Initial build flags --- augment with more flags later as needed
CFLAGS := -Wall -c -march=rv64g -mabi=lp64d -ffreestanding -nostdlib
# ASFLAGS := -march=rv64imafd_zicsr_zifencei -mabi=lp64d
LDFLAGS := -TLlinker.ld -static -nostdlib

############################################################################
# Avoid "crap" (.eh_frame and other debugging/unwinding sections) with:
#   -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-exceptions
# However, don't do this if debugging via gdb (use `make GDB=y').
############################################################################
ifdef GDB
 CFLAGS += -g
else
 CFLAGS += -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-exceptions
endif

############################################################################
# Non-PIC PC-relative relocation vs PIC/GOT
# In RISC-V, the `la rd,label' pseudo-instruction will be translated
# differently by the assembler based on whether -fno-pic (GAS default)
# or -fpic (gcc default) is passed to assembler (directly or via gcc),
# after which ld decides if the GOT is required. # Setting this explicitly
# will avoid confusion.  We will default to no-PIC/no-GOT + -no-pie,
# but run as `make PIC=y' to switch to PIC/GOT.
############################################################################
ifdef PIC
 CFLAGS += -fpic
 #ASFLAGS += -fpic
else
 CFLAGS += -fno-pic
 #ASFLAGS += -fno-pic
 LDFLAGS += -no-pie
endif

############################################################################
### Frame pointer will mess implementing fork without virtual memory!
############################################################################
ifdef FRAME_PTR
 CFLAGS += -fno-omit-frame-pointer
else
 CFLAGS += -fomit-frame-pointer
endif

.SUFFIXES:

all: $(LIB)

lib: $(LIB)

# Shell commands aka "recipes" (but nothing else) should begin with tab

## Static pattern rules for replacing builtin/implicit rule:
$(LIB) : $(LOBJS)
	ar rcs $(LIB) $(LOBJS)
$(COBJS) : %.o : %.c
	gcc $(CFLAGS) -o $@ $<
$(SOBJS) : %.o : %.S
	gcc $(CFLAGS) -o $@ $<
# See Secs 4.9, 4.10, Chapter 10 (esp 10.5) of GNU Make manual for Make v3.77

.PHONY: clean objclean
clean:
	rm -f $(OBJS) $(LIB)
objclean:
	rm -f $(OBJS)

