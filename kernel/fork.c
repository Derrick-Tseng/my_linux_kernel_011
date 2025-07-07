#include <errno.h>
#include <asm/system.h>
#include <linux/sched.h>

int find_empty_process(){
    int i = 0;
    // task[0] is occupied by the init process
    for(int i=1; i<NR_TASKS; i++){
        if(!task[i]){
            return i;
        }
    }
    return -EAGAIN; // No empty process found
}