GCC := x86_64-elf-gcc
LD := x86_64-elf-ld -m elf_i386
CCFLAG := -m32 -I../include -nostdinc -Wall -fomit-frame-pointer -c
LDFLAG := -Ttext 0x0 -s --oformat binary -m elf_i386
INCDIR := ../include
OBJS   := head.o main.o sched.o printk.o sys_call.o vsprintf.o traps.o fork.o asm.o panic.o chr_drv/chr_drv.a ../mm/mm.o ../lib/lib.a 

system: $(OBJS)
	$(LD) $(LDFLAG) -e startup_32 -o $@ $^ -M > System.map

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

../mm/mm.o : ../mm/*.c
	cd ../mm; make mm.o; cd ..

sys_call.o : sys_call.S
	$(GCC) -m32 -traditional -c -o $@ $<

traps.o : traps.c
	$(GCC) $(CCFLAG) -o $@ $<

asm.o : asm.S
	$(GCC) -m32 -traditional -c -o $@ $<

fork.o : fork.c
	$(GCC) $(CCFLAG) -o $@ $<

panic.o : panic.c
	$(GCC) $(CCFLAG) -o $@ $<

clean :
	rm -f *.o
	rm -f system
	rm -f System.map
	cd chr_drv; make clean; cd ..
	cd ../mm; make clean; cd ../kernel
	cd ../lib; make clean; cd ../kernel