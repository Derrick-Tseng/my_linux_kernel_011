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
    if((--current->counter) > 0)
        return; // If the counter is still positive, return

    current->counter = 0;

    // Check if the process is in the kernel mode or user mode
    // Do not schedule if the process is in kernel mode
    if(!cpl)
        return;

    schedule();
}

void schedule() {
    int i, next, c;
    struct task_struct **p;

    while(1){
        c = -1;
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];

        while(--i){
            if(!*--p)
                continue; // Skip if task slot is empty

            // If the task is runnable and has a highest counter(time slice)
            if((*p)->state == TASK_RUNNING && (*p)->counter > c){
                c = (*p)->counter;
                next = i;
            }
        }

        if(c)
            break;

        // If no task available, update the counter for all tasks
        for(p=&LAST_TASK; p>&FIRST_TASK; p--){
            if(!*p)
                continue; // Skip if task slot is empty

            (*p)->counter = ((*p)->priority >> 1) + (*p)->priority;
        }
    }
    switch_to(next); // Switch to the selected task
}

static inline void __sleep_on(struct task_struct **p, int state){
    // Think of *p as the "wait queue."
    // **p points to a wait queue head

    struct task_struct *tmp;

    // If the wait queue pointer is null, there's nothing to sleep on
    if(!p)
        return;

    // INIT process should not sleep
    if(current == &(init_task.task))
        panic("init_task going to sleep");

    // Save the task that was previously at the head of the wait queue.
    tmp = *p;

    // Makes the current task the new head
    *p = current;
    current->state = state;

repeat:
    schedule();

    //  If another task has become the head of the wait queue (*p != current) while this process was sleeping.
    //  It means this task needs to keep waiting, so it sets itself to TASK_UNINTERRUPTIBLE and loops back to schedule again.
    if(*p && *p != current){
        // The new task at the head of the queue (*p) is woken up
        (**p).state = 0;
        // The current process puts itself back to sleep.
        current->state = TASK_UNINTERRUPTIBLE;
        // Let the newly awakened task run.
        goto repeat;
    }

    if(!*p)
        printk("Warning: *p = NULL\n\r");

    // Once the task is at the head of the queue (meaning it's been woken up)
    // Removes itself from the wait queue by restoring the original head of the queue.
    *p = tmp;

    // If there was another task in the queue, wake it up.
    // This creates a "chain reaction" to wake up all tasks in the queue.
    if(*p)
        tmp->state = 0;
}

void interruptible_sleep_on(struct task_struct** p) {
    __sleep_on(p, TASK_INTERRUPTIBLE);
}

void sleep_on(struct task_struct** p) {
    __sleep_on(p, TASK_UNINTERRUPTIBLE);
}

void wake_up(struct task_struct **p) {
    if (p && *p) {
        if ((**p).state == TASK_STOPPED)
            printk("wake_up: TASK_STOPPED");
        if ((**p).state == TASK_ZOMBIE)
            printk("wake_up: TASK_ZOMBIE");
        (**p).state=0;
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