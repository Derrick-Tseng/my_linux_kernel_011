#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/kernel.h> 

void keyboard_handler(void){
    unsigned char a, scan_code;

    scan_code = inb_p(0x60);
    outb_p(0x20, 0x20);
    printk("%x\n\r", (int)scan_code);
    return;
}