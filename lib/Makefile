AR  := x86_64-elf-ar
GCC := x86_64-elf-gcc
LD := x86_64-elf-ld
CCFLAG := -m32 -I../include -nostdinc -ffreestanding -fno-pic -Wall -fomit-frame-pointer -fno-stack-protector -c
OBJS   := string.o

lib.a : $(OBJS)
		$(AR) rcs $@ $^
		sync

string.o : string.c
		$(GCC) $(CCFLAG) -o $@ $<

clean : 
		-rm *.o
		-rm lib.a

