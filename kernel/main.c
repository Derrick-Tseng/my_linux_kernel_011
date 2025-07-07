#define __LIBRARY__
#define EXT_MEM_K (*(unsigned short *)0x90002) // Extended memory in KB, read from BIOS data area

#include <linux/tty.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/system.h>

extern void mem_init(long start_mem, long end_mem);

static long memory_end = 0;
static long buffer_memory_end = 0;
static long main_memory_start = 0;

void main(void) {

    // 1MB + extended memory in KB
    // Left shift the value of EXT_MEM_K by 10 bits to convert it from kilobytes to bytes
    memory_end = (1 << 20) + (EXT_MEM_K << 10);
    memory_end &= 0xFFFFF000; // Align to 64KB boundary

    if(memory_end > 16 * 1024 * 1024){
        memory_end = 16 * 1024 * 1024;
    }

    // 2nd to 4th MB is reserved for the DMA Buffer
    if(memory_end > 12 * 1024 * 1024){
        buffer_memory_end = 4 * 1024 * 1024;
    }
    else if(memory_end > 6 * 1024 * 1024){
        buffer_memory_end = 2 * 1024 * 1024;
    }
    else{
        buffer_memory_end = 1 * 1024 * 1024;
    }

    main_memory_start = buffer_memory_end;
    mem_init(main_memory_start, memory_end);

    trap_init();
    sched_init();

    tty_init();
    printk("\n\rmemory start: %d, end: %d\n\r", main_memory_start, memory_end);

    move_to_user_mode();

    while (1) {
        test_a();
    }
}