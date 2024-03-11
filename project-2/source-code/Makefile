SRCS := $(wildcard *.c)
default: all

run:
	@for f in $(SRCS); do echo "Compiling $$f..."; gcc -Wall -c $$f ; done
	@ld -T Llinker.ld -static -nostdlib -o Lcli Lcli.o Lbio.o walkfunctions.o Ldiskio.o lib4490.a
	@echo "Done. Run the code with ./Lcli"

start:
	@./Lcli ../mitfs/fs.img

all:
	make run && make start
