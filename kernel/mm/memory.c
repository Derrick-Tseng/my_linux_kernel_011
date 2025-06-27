#include <linux/sched.h>

unsigned long HIGH_MEMORY = 0x100000; // 1MB, the end of low memory
unsigned char mem_map[PAGING_PAGES] = {0, }; // Memory map for pages

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