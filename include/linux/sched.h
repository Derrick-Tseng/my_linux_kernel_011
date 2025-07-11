#ifndef _SCHED_H
#define _SCHED_H

// Linux 0.11 supports only 64 tasks
#define NR_TASKS 64
#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#define HZ 100

#define TASK_SIZE       0x04000000

#if (TASK_SIZE & 0x3fffff)
#error "TASK_SIZE must be multiple of 4M"
#endif

#if (((TASK_SIZE>>16)*NR_TASKS) != 0x10000)
#error "TASK_SIZE*NR_TASKS must be 4GB"
#endif

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_ZOMBIE             3
#define TASK_STOPPED            4

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_ZOMBIE             3
#define TASK_STOPPED            4

#include <linux/head.h>
#include <linux/mm.h>

extern int copy_page_tables(unsigned long from, unsigned long to, long size);
extern int free_page_tables(unsigned long from, unsigned long size);

void trap_init();
void sched_init();

void test_a();
void test_b();
extern void schedule();
extern void panic(const char* s);

extern struct task_struct *task[NR_TASKS];
extern struct task_struct *current;

typedef int (*fn_ptr)();

struct tss_struct {
    long back_link;
    long esp0;
    long ss0;
    long esp1;
    long ss1;
    long esp2;
    long ss2;
    long cr3;
    long eip;
    long eflags;
    long eax, ecx, edx, ebx;
    long esp;
    long ebp;
    long esi;
    long edi;
    long es;
    long cs;
    long ss;
    long ds;
    long fs;
    long gs;
    long ldt;
    long trace_bitmap;
};

struct task_struct {
    long state;
    long counter;
    long priority;
    long pid;
    struct task_struct *p_pptr;
    struct desc_struct ldt[3];
    struct tss_struct tss;
};

#define INIT_TASK \
{                   \
    0,              \
    15,             \
    15,             \
    0,              \
    &init_task.task,\
    {               \
        {0, 0},     \
        {0xfff, 0xc0fa00},   \
        {0xfff, 0xc0f200},   \
    },              \
    {0, PAGE_SIZE + (long)&init_task, 0x10, 0, 0, 0, 0, (long)&pg_dir, \
        0, 0, 0, 0, 0, 0, 0, 0, \
        0, 0, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,   \
        _LDT(0), 0x80000000,    \
    },              \
}

/*
 * In linux is 4, because we add video selector,
 * so, it is 5 here.
 * */
#define FIRST_TSS_ENTRY 5
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY + 1)
#define _TSS(n) ((((unsigned long)n) << 4) + (FIRST_TSS_ENTRY << 3))
#define _LDT(n) ((((unsigned long)n) << 4) + (FIRST_LDT_ENTRY << 3))
#define ltr(n) __asm__("ltr %%ax"::"a"(_TSS(n)))
#define lldt(n) __asm__("lldt %%ax"::"a"(_LDT(n)))


// Asm Line 1: Compare current task with the new task
// Asm Line 4: points current to the new task
// Asm Line 5: Jump to the new task's code segment and the cpu saves the context to the current state's TSS
#define switch_to(n) {\
    struct {long a,b;} __tmp; \
    __asm__("cmpl %%ecx,current\n\t" \
            "je 1f\n\t" \
            "movw %%dx,%1\n\t" \
            "xchgl %%ecx,current\n\t" \
            "ljmp *%0\n\t" \
            "1:" \
            ::"m" (*&__tmp.a),"m" (*&__tmp.b), \
            "d" (_TSS(n)),"c" ((long) task[n])); \
}

#define _set_base(addr,base) \
__asm__("movw %%dx,%0\n\t" \
        "rorl $16,%%edx\n\t" \
        "movb %%dl,%1\n\t" \
        "movb %%dh,%2" \
        ::"m" (*((addr)+2)), \
        "m" (*((addr)+4)), \
        "m" (*((addr)+7)), \
        "d" (base) \
        :)

#define _set_limit(addr,limit) \
__asm__("movw %%dx,%0\n\t" \
        "rorl $16,%%edx\n\t" \
        "movb %1,%%dh\n\t" \
        "andb $0xf0,%%dh\n\t" \
        "orb %%dh,%%dl\n\t" \
        "movb %%dl,%1" \
        ::"m" (*(addr)), \
        "m" (*((addr)+6)), \
        "d" (limit) \
        :"dx")

#define set_base(ldt,base) _set_base( ((char *)&(ldt)) , base )
#define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )

#define _get_base(addr) ({\
unsigned long __base; \
__asm__("movb %3,%%dh\n\t" \
    "movb %2,%%dl\n\t" \
    "shll $16,%%edx\n\t" \
    "movw %1,%%dx" \
    :"=d" (__base) \
    :"m" (*((addr)+2)), \
    "m" (*((addr)+4)), \
    "m" (*((addr)+7))); \
__base;})

#define get_base(ldt) _get_base( ((char *)&(ldt)) )

#define get_limit(segment) ({ \
unsigned long __limit; \
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})

#endif
