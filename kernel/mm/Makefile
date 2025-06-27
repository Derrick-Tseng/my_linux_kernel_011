AR  := x86_64-elf-ar
LD  := x86_64-elf-ld
GCC := x86_64-elf-gcc
CCFLAG := -m32 -I../../include -nostdinc -ffreestanding -fno-pic -Wall -fomit-frame-pointer -fno-stack-protector -c

OBJS   := memory.o

mm.o : $(OBJS)
	$(LD) -m elf_i386 -r -o $@ $<

memory.o : memory.c
	$(GCC) $(CCFLAG) -o $@ $<

clean :
	rm *.o
