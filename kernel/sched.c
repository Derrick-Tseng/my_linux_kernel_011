#include <errno.h>
#include <linux/sched.h>
#include <string.h>
#include <linux/kernel.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/fork.h>
#include <linux/sys.h>
 
#define COUNTER 100

#define PAGE_SIZE 4096
#define LATCH (1193180/HZ)

extern int system_call();
extern void timer_interrupt();

// pcb
union task_union {
    struct task_struct task;
    char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK, };

// Always point to the current task
// The current task is the init task
struct task_struct *current = &(init_task.task);

// The first element is set to point to the task member of the init_task union
// The rest are automatically initialized to NULL
struct task_struct *task[NR_TASKS] = {&(init_task.task), };


long user_stack[PAGE_SIZE >> 2];

struct
{
    long *a;
    short b;
} stack_start = {&user_stack[PAGE_SIZE >> 2], 0x10};


int clock = COUNTER;
static int cnt = 0;
static int isFirst = 1;
void do_timer(long cpl) {
    if (clock >0 && clock <= COUNTER) {
        clock--;
    }
    else if (clock == 0) {
        schedule();
    }
    else {
        clock = COUNTER;
    }
}

void schedule() {
    if (current == task[0] && task[1]) {
        switch_to(1);
    }
    else if (current == task[1]) {
        switch_to(0);
    }
}


void sched_init() {
    int i;
    struct desc_struct* p;
    // Sets up the Task State Segment (TSS) descriptor for the initial task
    set_tss_desc(gdt + FIRST_TSS_ENTRY, &(init_task.task.tss));
    // Sets up the Local Descriptor Table (LDT) descriptor for the initial task
    set_ldt_desc(gdt + FIRST_LDT_ENTRY, &(init_task.task.ldt));

    // Initializes all task slots in the task array to NULL
    // Clears all TSS and LDT descriptor entries in the Global Descriptor Table (GDT)
    p = gdt+2+FIRST_TSS_ENTRY;
    for(i=0; i<NR_TASKS; i++, p+=2){
        task[i] = 0;
        p->a = 0;
        p->b = 0;
        p++;
        p->a = 0;
        p->b = 0;
        p++;
    }

    // Clears the Nested Task (NT) flag in EFLAGS register
    __asm__("pushfl; andl $0xffffbfff, (%esp); popfl");

    // Loads the Task Register (TR) and LDT Register with initial values
    ltr(0);
    lldt(0);

    // Set up the timer
    outb_p(0x36, 0x43);
    outb_p(LATCH & 0xff, 0x40);
    outb(LATCH >> 8, 0x40);

    // Sets up the Interrupt Descriptor Table (IDT) for the timer interrupt
    set_intr_gate(0x20, &timer_interrupt);
    outb(inb_p(0x21) & ~0x01, 0x21);

    // Sets up system call gate for interrupt 0x80
    set_system_gate(0x80, &system_call);
}


void test_a(void) {
__asm__("movl $0x0, %edi\n\r"
        "movw $0x1b, %ax\n\t"
        "movw %ax, %gs \n\t"
        "movb $0x0c, %ah\n\r"
        "movb $'A', %al\n\r"
        "loopa:\n\r"
        "movw %ax, %gs:(%edi)\n\r"
        "jmp loopa");
}


void test_b(void) {
__asm__("movl $0x30, %edi\n\r"
        "movw $0x1b, %ax\n\t"
        "movw %ax, %gs \n\t"
        "movb $0x0c, %ah\n\r"
        "movb $'B', %al\n\r"
        "loopb:\n\r"
        "movw %ax, %gs:(%edi)\n\r"
        "jmp loopb");
}