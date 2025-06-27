GCC := x86_64-elf-gcc
LD := x86_64-elf-ld -m elf_i386
CCFLAG := -m32 -I../include -nostdinc -Wall -fomit-frame-pointer -c
LDFLAG := -Ttext 0x0 -s --oformat binary -m elf_i386
INCDIR := ../include
OBJS   := head.o main.o sched.o chr_drv/chr_drv.a printk.o vsprintf.o sys_call.o mm/mm.o ../lib/lib.a

system: $(OBJS)
	$(LD) $(LDFLAG) -e startup_32 -o $@ $^

head.o : head.S
	$(GCC) -m32 -traditional -c -o $@ $<

sched.o : sched.c
	$(GCC) $(CCFLAG) -o $@ $<

chr_drv/chr_drv.a: chr_drv/*.c
	cd chr_drv; make chr_drv.a; cd ..

printk.o : printk.c
	$(GCC) $(CCFLAG) -o $@ $<

vsprintf.o : vsprintf.c
	$(GCC) $(CCFLAG) -o $@ $<

../lib/lib.a : ../lib/*.c
	cd ../lib; make lib.a; cd ../kernel

main.o : main.c
	$(GCC) $(CCFLAG) -o $@ $<

mm/mm.o : mm/*.c
	cd mm; make mm.o; cd ..

sys_call.o : sys_call.S
	$(GCC) -m32 -traditional -c -o $@ $<

clean :
	rm -f *.o
	rm -f system
	cd chr_drv; make clean; cd ..
	cd ../lib; make clean; cd ../kernel