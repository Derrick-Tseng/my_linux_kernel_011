LD  := x86_64-elf-ld
GCC := x86_64-elf-gcc
CCFLAG := -m32 -I../include -nostdinc -ffreestanding -fno-pic -Wall -fomit-frame-pointer -fno-stack-protector -c
LDFLAG := -Ttext 0x0 -s --oformat binary -m elf_i386
INCDIR := ../include
OBJS   := memory.o swap.o page.o

mm.o : $(OBJS)
	$(LD) -m elf_i386 -r -o $@ $^

memory.o : memory.c
	$(GCC) $(CCFLAG) -o $@ $<

swap.o : swap.c
	$(GCC) $(CCFLAG) -o $@ $<

page.o : page.S
	$(GCC) -m32 -traditional -c -o $@ $<

clean :
	rm -f *.o