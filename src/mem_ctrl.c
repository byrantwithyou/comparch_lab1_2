/*
* Yunxin Sun 2020 
*/

/*
* Procedure:Mem_Cycle()
* Purpose: Main Memory Cycle
*/

#include "cache.h"
#include "shell.h"

void mem_cycle() {
    for (int i = 0; i < 16; ++i) {
        l2_cache.mshr[i].valid = FALSE;
    }
}
