AR  := x86_64-elf-ar
LD := x86_64-elf-ld
GCC := x86_64-elf-gcc
CCFLAG := -m32 -I../../include -nostdinc -ffreestanding -fno-pic -Wall -fomit-frame-pointer -fno-stack-protector -c
OBJS   := tty_io.o console.o

tty_io.o : tty_io.c
	$(GCC) $(CCFLAG) -o $@ $<

console.o : console.c
	$(GCC) $(CCFLAG) -o $@ $<

chr_drv.a : $(OBJS)
	$(AR) rcs $@ $^
	sync

clean : 
	rm -f *.o
	rm -f chr_drv.a

