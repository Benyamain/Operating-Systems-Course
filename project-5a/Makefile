##################################################################
# Makefile for building MT system over riscv64
##################################################################

##################################################################
# About tabs/spaces in Makefiles:
# A leading TAB means the line is an external command to be run by the shell!
# So:
# - Any external shell command (aka "recipe") line must begin with TAB
# - No other line in the makefile should begin with a TAB!
##################################################################

# Assembly sources:
ASRCS := ts-riscv64.S
# Non-main functions C sources:
HSRCS := queue.c user.c
# Main kernel program (in C):
ESRCS := kernel.c
CSRCS := $(HSRCS) $(ESRCS)

SOBJS := $(ASRCS:%.S=%.o)
HOBJS := $(HSRCS:%.c=%.o)
EOBJS := $(ESRCS:%.c=%.o)
COBJS := $(HOBJS) $(EOBJS)
OBJS := $(COBJS) $(SOBJS)
EXECS := $(ESRCS:%.c=%)

## The Llibc "pico C library" is in the ./Llibc subdirectory
SUBDIR := ./Llibc
LIB := $(SUBDIR)/lib4490.a

# Initial build flags --- augment with more flags later as needed
CFLAGS := -Wall -c -march=rv64g -mabi=lp64d -ffreestanding -nostdlib
# ASFLAGS := -march=rv64imafd_zicsr_zifencei -mabi=lp64d
LDFLAGS := -TLlinker.ld -static -nostdlib

############################################################################
# Avoid .eh_frame/debugging/unwinding sections and other "crap" with:
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
# after which ld decides if the GOT is required.  Setting this explicitly
# will avoid confusion.  We will default to no-PIC/no-GOT + -no-pie
# Run as `make PIC=y' to switch to PIC/GOT.
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
### When implementing standard unix fork without virtual memory,
### the frame pointer cannot be used!
############################################################################
ifdef FRAME_PTR
 CFLAGS += -fno-omit-frame-pointer
else
 CFLAGS += -fomit-frame-pointer
endif

.SUFFIXES:

all: $(LIB) $(EXECS)

lib: $(LIB)

$(LIB): $(SUBDIR)

$(SUBDIR):
	$(MAKE) -C $@

.PHONY: all $(SUBDIR)

# Shell commands aka "recipes" (but nothing else) should begin with tab

## Static pattern rules for replacing builtin/implicit rule:
$(EXECS) : % : %.o $(HOBJS) $(SOBJS) $(LIB) 
	ld $(LDFLAGS) -o $@ $^
$(COBJS) : %.o : %.c
	gcc $(CFLAGS) -o $@ $<
$(SOBJS) : %.o : %.S
	gcc $(CFLAGS) -o $@ $<
# See Secs 4.9, 4.10, Chapter 10 (esp 10.5) of GNU Make manual for Make v3.77

.PHONY: clean objclean $(CLEANSUBDIR)

CLEANSUBDIR = $(addprefix clean-,$(SUBDIR))

OBJCLEANSUBDIR = $(addprefix objclean-,$(SUBDIR))

$(CLEANSUBDIR): clean-%:
	$(MAKE) -C $* clean

$(OBJCLEANSUBDIR): objclean-%:
	$(MAKE) -C $* objclean

clean: $(CLEANSUBDIR)
	rm -f $(EXECS) $(OBJS) $(LIB)

objclean: $(OBJCLEANSUBDIR)
	rm -f $(OBJS)

# checking the code for kernel.c and queue.c
# drop each of the programs as you are implementing for each file
# so by the end, you should have only the make commands
#	make clean
#	cp ./pre-compiled/kernel.o .      # Drop this once you finish kernel.c
#	cp ./pre-compiled/queue.o .       # Drop this once you finish queue.c
#	make
