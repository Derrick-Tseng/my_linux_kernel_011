#define __LIBRARY__
#define EXT_MEM_K (*(unsigned short *)0x90002) // Extended memory in KB, read from BIOS data area

#include <linux/tty.h>
#include <linux/kernel.h>
#include <linux/sched.h>

extern void mem_init(long start_mem, long end_mem);

static long memory_end = 0;
static long buffer_memory_end = 0;
static long main_memory_start = 0;

void main(void) {
    memory_end = (1 << 20) + (EXT_MEM_K << 10); // 1MB + extended memory in KB
    memory_end &= 0xFFFFF000; // Align to 64KB boundary

    // Left shift the value of EXT_MEM_K by 10 bits to convert it from kilobytes to bytes
    memory_end = (1<<20) + (EXT_MEM_K<<10);
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
    printk("memory start: %d, end: %d", main_memory_start, memory_end);
    __asm__ __volatile__(
            "int $0x80\n\r"
            "movw $0x1b, %%ax\n\r"
            "movw %%ax, %%gs\n\r"
            "movl $0, %%edi\n\r"
            "movw $0x0f41, %%gs:(%%edi)\n\r"
            "loop:\n\r"
            "jmp loop"
            ::);
}