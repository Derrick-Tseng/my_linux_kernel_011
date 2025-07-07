#ifndef _MM_H
#define _MM_H

#define PAGE_SIZE 4096 // 4KB page size

extern unsigned long get_free_page();

#define LOW_MEM 0x100000
extern unsigned long HIGH_MEMORY;
// Only 15MB of memory is used for paging because the 1st MB is reserved for the kernel
#define PAGING_MEMORY (15*1024*1024)
#define PAGING_PAGES (PAGING_MEMORY>>12)
// Converts a physical address to an index in the page management arrays
#define MAP_NR(addr) (((addr)-LOW_MEM)>>12)
#define USED  100

extern unsigned char mem_map [ PAGING_PAGES ];

#endif

