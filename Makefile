AS := x86_64-elf-as --32
LD := x86_64-elf-ld -m elf_i386
QEMU = qemu-system-i386
QEMU_OPTS = -drive file=linux.img,format=raw,if=floppy -boot a -monitor stdio

LDFLAG := -Ttext 0x0 -s --oformat binary

image : linux.img

linux.img : tools/build bootsect setup kernel/system
	./tools/build bootsect setup kernel/system > $@

tools/build : tools/build.c
		gcc -o $@ $<

kernel/system : kernel/head.S kernel/*.c
		cd kernel; make system; cd ..

bootsect : bootsect.o
	$(LD) $(LDFLAG) -o $@ $<

bootsect.o : bootsect.S
	$(AS) -o $@ $<

setup : setup.o
		$(LD) $(LDFLAG) -e _start_setup -o $@ $<

setup.o : setup.S
		$(AS) -o $@ $<

run: linux.img
	# $(QEMU) -drive file=$<,format=raw,if=floppy -boot a
	$(QEMU) $(QEMU_OPTS)

clean:
	rm -f *.o
	rm -f bootsect
	rm -f setup
	rm -f tools/build
	rm -f linux.img
	cd kernel; make clean; cd ..