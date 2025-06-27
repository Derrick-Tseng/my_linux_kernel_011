#include <linux/sched.h>
#include <asm/system.h>

#define PAGE_SIZE 4096

extern int system_call();

long user_stack[PAGE_SIZE >> 2]; // 1024 long words, 4096 bytes

struct {
    long *a;
    short b;
} stack_start = {&user_stack[PAGE_SIZE >> 2], 0x10};

void sched_init() {
    set_intr_gate(0x80, &system_call);
}
