#include <errno.h>
#include <asm/system.h>
#include <linux/sched.h>
#include <string.h>

long last_pid = 0;

// This function should copy the memory space of the current process
// to the new process. For simplicity, we assume it returns 0 on success.
int copy_mem(int nr, struct task_struct *p){
    unsigned long old_data_base, new_data_base, data_limit;
    unsigned long old_code_base, new_code_base, code_limit;

    // retrieves the size limits for the code and data segments
    // The parameters are segment selector values.
    code_limit = get_limit(0x0f);
    data_limit = get_limit(0x17);

    // Get the base addresses of the code and data segments from the current process's LDT entries
    old_code_base = get_base(current->ldt[1]);
    old_data_base = get_base(current->ldt[2]);

    if(old_data_base != old_code_base){
        panic("Do not support different code and data base addresses");
    }
    if(data_limit < code_limit){
        panic("Data limit is less than code limit");
    }

    new_data_base = new_code_base = nr * TASK_SIZE;
    set_base(p->ldt[1], new_code_base);
    set_base(p->ldt[2], new_data_base);

    if(copy_page_tables(old_data_base, new_data_base, data_limit)){
        free_page_tables(new_data_base, data_limit);
        return -ENOMEM; // Memory allocation failed
    }

    return 0;
}

int copy_process(int nr, long ebp, long edi, long esi, long gs,
        long none, long ebx, long ecx,
        long edx, long orig_eax, long fs,
        long es, long ds, long eip, long cs, 
        long eflags, long esp, long ss){
    
    struct task_struct *p;
    p = (struct task_struct *)get_free_page();
    if(!p){
        return -EAGAIN; // No free page available
    }

    task[nr] = p;
    memcpy(p, current, sizeof(struct task_struct));

    // Set the process ID for the new process
    p->pid = last_pid;
    // Set the parent pointer to the current process
    p->p_pptr = current;

    // Initialize Task State Segment (TSS) for the new process
    p->tss.back_link = 0;                    // No previous task
    
    // Set the kernel stack pointer to the top of the allocated page
    p->tss.esp0 = PAGE_SIZE + (long)p;       // Kernel stack pointer
    p->tss.ss0 = 0x10;                       // Kernel stack segment selector
    p->tss.cr3 = current->tss.cr3;           // Copy page directory from parent
    
    // Set up user mode register values from system call entry
    p->tss.eip = eip;                        // Instruction pointer
    p->tss.eflags = eflags;                  // Processor flags
    p->tss.eax = 0;                          // Return 0 from fork() in child
    p->tss.ecx = ecx;                        // General purpose register
    p->tss.edx = edx;                        // General purpose register
    p->tss.ebx = ebx;                        // General purpose register
    p->tss.esp = esp;                        // User stack pointer
    p->tss.ebp = ebp;                        // Base pointer
    p->tss.esi = esi;                        // Source index register
    p->tss.edi = edi;                        // Destination index register
    
    // Set up segment selectors (mask to get only selector bits)
    p->tss.es = es & 0xffff;                 // Extra segment
    p->tss.cs = cs & 0xffff;                 // Code segment
    p->tss.ss = ss & 0xffff;                 // Stack segment
    p->tss.ds = ds & 0xffff;                 // Data segment
    p->tss.fs = fs & 0xffff;                 // Additional segment
    p->tss.gs = gs & 0xffff;                 // Additional segment
    
    p->tss.ldt = _LDT(nr);                   // Local Descriptor Table selector
    p->tss.trace_bitmap = 0x80000000;        // I/O permission bitmap offset

    if(copy_mem(nr, p)){
        task[nr] = NULL;
        free_page((long)p);
        return -EAGAIN; // Memory copy failed
    }

    set_tss_desc(gdt + (nr << 1) + FIRST_TSS_ENTRY, &(p->tss));
    set_ldt_desc(gdt + (nr << 1) + FIRST_LDT_ENTRY, &(p->ldt));

    return last_pid;

}


int find_empty_process() {
    int i;
repeat:
    // Increment PID and wrap to 1 if it becomes negative (overflow protection)
    if ((++last_pid)<0) last_pid=1;

    // Check if the new PID is already in use by any existing process
    for(i=0 ; i<NR_TASKS ; i++) {
        if (task[i] && (task[i]->pid == last_pid))
            goto repeat; // PID collision, try next PID
    }

    // task[0] is occupied by the init process
    for (i = 1; i < NR_TASKS; i++) {
        if (!task[i])
            return i; // Return the empty slot index
    }

    return -EAGAIN; // No empty process slot found
}