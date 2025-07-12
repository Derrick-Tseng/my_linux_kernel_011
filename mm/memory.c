#include <linux/sched.h>

unsigned long HIGH_MEMORY = 0x100000; // 1MB, the end of low memory
unsigned char mem_map[PAGING_PAGES] = {0, }; // Memory map for pages

void free_page(unsigned long addr){
    if(addr < LOW_MEM)
        return; // Cannot free memory below LOW_MEM

    if(addr >= HIGH_MEMORY)
        panic("Trying to free memory above HIGH_MEMORY");

    addr -= LOW_MEM;
    addr >>= 12; // Convert to page number
    if(mem_map[addr]--) // Decrement the page count if it was allocated
        return;

    mem_map[addr] = 0; // Mark the page as free
    panic("Trying to free an unallocated page");
}

int free_page_tables(unsigned long from,unsigned long size) {
    unsigned long *pg_table;
    unsigned long *dir, nr;

    if(from & 0x3fffff)
        panic("free_page_tables called with unaligned address");
    if(!from)
        panic("Trying to free up swapper memory space");
    
    size = (size + 0x3fffff) >> 22; // Convert a byte size into the number of 4MB page directories
    dir = (unsigned long *)((from >> 20) & 0xffc);

    for(; size-- > 0; dir++){
        if(!(1 & *dir))
            continue; // No page table to free

        pg_table = (unsigned long *)(0xfffff00 & *dir);
        for(nr = 0; nr < 1024; nr++){
            if(*pg_table){
                if(1 & *pg_table)
                    free_page(0xfffff000 & *pg_table); // Free the page
                *pg_table = 0; // Clear the page table entry
            }
            pg_table++; // Move to the next page table entry
        }
        free_page(0xfffff000 & *dir); // Free the page table itself
        *dir = 0; // Clear the directory entry
    }
    invalidate();
    return 0;
}

int copy_page_tables(unsigned long from,unsigned long to,long size) {
    unsigned long *from_page_table;
    unsigned long *to_page_table;
    unsigned long this_page;
    unsigned long *from_dir, *to_dir;
    unsigned long nr;

    // Is align to 4KB boundary
    if((from & 0x3fffff) || (to & 0x3fffff))
        panic("copy_page_tables called with unaligned address");

    // Right shift 22 bits to get the page directory index
    // Left shift 2 bits to get the page table index (4 bit for each PDE)
    from_dir = (unsigned long *)((from >> 20) & 0xffc);
    to_dir = (unsigned long *)((to >> 20) & 0xffc);

    // Calculate the number of PDE
    size = (unsigned)(size + 0x3fffff) >> 22;
    for(; size-- > 0; from_dir++, to_dir++){
        if(1 & *to_dir) 
            panic("copy_page_tables called with already present page table");

        if(!(1 & *from_dir)) 
            continue; // No page table to copy

        from_page_table = (unsigned long *)(0xfffff00 & *from_dir);
        if(!(to_page_table = (unsigned long *)get_free_page())) 
            return -1; // No free page available

        // Set present, read/write, user access
        *to_dir = ((unsigned long) to_page_table) | 7;

        // If the parent process is INIT, copy only page tables in the first 640KB memory
        // Otherwise, copy the entire page table
        nr = (from == 0) ? 0xa0 : 1024;

        for(; nr-- > 0; from_page_table++, to_page_table++){
            this_page = *from_page_table;
            if(!this_page) 
                continue; // No page to copy

            if(!(1 & this_page)) 
                continue; // Not a valid page

            // Set read/write to 0
            this_page &= ~2;
            *to_page_table = this_page; // Copy the page

            // Copy on write only if the page is above LOW_MEM(1MB)
            // Set the parent process's page table entry to read-only
            if(this_page > LOW_MEM){
                *from_page_table = this_page;
                this_page -= LOW_MEM;
                this_page >>= 12; // Convert to page number
                mem_map[this_page]++;
            }
        }
    }
    invalidate();
    return 0;
}

void mem_init(long start_mem, long end_mem){
    int i;
    HIGH_MEMORY = end_mem;

    for(int i=0; i<PAGING_PAGES; i++){
        mem_map[i] = USED; // Mark all pages as used
    }

    i = MAP_NR(start_mem);
    end_mem -= start_mem;
    end_mem >>= 12; // Convert to pages
    while(end_mem--){
        mem_map[i++] = 0; // Mark pages as free
    }

}