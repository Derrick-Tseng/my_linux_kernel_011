#include <errno.h>
#include <string.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fork.h>
#include <asm/system.h>
#include <asm/io.h>

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

void do_timer(long cpl) {
    static unsigned char c = '0';
    if (c > 127) {
        c = '0';
    }
    printk("\b%c", c++);
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

    create_second_process();

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

int create_second_process(){
    struct task_struct *p;
    int i, nr;

    nr = find_empty_process();
    if(nr < 0){
        return  -EAGAIN;
    }

    p = (struct task_struct *) get_free_page();
    memcpy(p, current, sizeof(struct task_struct));

    set_tss_desc(gdt + (nr << 1) + FIRST_TSS_ENTRY, &(p->tss));
    set_ldt_desc(gdt + (nr << 1) + FIRST_LDT_ENTRY, &(p->ldt));

    memcpy(&p->tss, &current->tss, sizeof(struct tss_struct));

    p->tss.eip = (long)test_b;
    p->tss.ldt = _LDT(nr);
    p->tss.ss0 = 0x10;
    p->tss.esp0 = PAGE_SIZE + (long)p;
    p->tss.ss  = 0x10;
    p->tss.ds  = 0x10;
    p->tss.es  = 0x10;
    p->tss.cs  = 0x8;
    p->tss.fs  = 0x10;
    p->tss.esp = PAGE_SIZE + (long)p;
    p->tss.eflags = 0x602;

    task[nr] = p;
    return nr;
}

void test_a(void) {
__asm__("movl $0, %edi\n\r"
        "movl $0x17, %eax\n\t"
        "movw %ax, %ds \n\t"
        "movw %ax, %es \n\t"
        "movw %ax, %fs \n\t"
        "movw $0x18, %ax\n\t"
        "movw %ax, %gs \n\t"
        "movb $0x0c, %ah\n\r"
        "movb $'A', %al\n\r"
        "loopa:\n\r"
        "movw %ax, %gs:(%edi)\n\r"
        "jmp loopa");
}

void test_b(void) {
__asm__("movl $0, %edi\n\r"
        "movw $0x18, %ax\n\t"
        "movw %ax, %gs \n\t"
        "movb $0x0f, %ah\n\r"
        "movb $'B', %al\n\r"
        "loopb:\n\r"
        "movw %ax, %gs:(%edi)\n\r"
        "jmp loopb");
}