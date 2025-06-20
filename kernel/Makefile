GCC := x86_64-elf-gcc
LD := x86_64-elf-ld -m elf_i386
CCFLAG := -I../include -nostdinc -Wall -fomit-frame-pointer -c
LDFLAG := -Ttext 0x0 -s --oformat binary -m elf_i386
INCDIR := ../include
OBJS   := head.o main.o 

system: $(OBJS)
	$(LD) $(LDFLAG) -e startup_32 -o $@ $^

head.o : head.S
	$(GCC) -m32 -traditional -c -o $@ $<

main.o : main.c
	$(GCC) -m32 $(CCFLAG) -o $@ $<

clean : 
	rm -f *.o
	rm -f system