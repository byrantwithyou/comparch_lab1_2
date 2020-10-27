/*
* Yunxin Sun 2020 
*/

/*
* Procedure:Mem_Cycle()
* Purpose: Main Memory Cycle
*/

#include "cache.h"
#include "shell.h"
#include <string.h>
#include <assert.h>

/*
* Procedure: mem_cycle()
* Purpose: main memory cycle
*/
void mem_cycle() {
    l2_cache.mshr.length = 0;
    for (int i = 0; i < 16; ++i) {
        l2_cache.mshr.mshr_arr[i].valid = FALSE;
    }
}

/* ================= Operation for mshr_queue ================= */

/*
* Procedure: init_mshr()
* Purpose: initialize the mshr queue
*
*/
void init_mshr() {
    l2_cache.mshr.length = 0;
    memset(l2_cache.mshr.mshr_arr, 0, sizeof l2_cache.mshr.mshr_arr);
}

/*
* Procedure: in_mshr
* Purpose: add a memory request into the queue
*
*/
void in_mshr(uint32_t address) {
    int *length = &(l2_cache.mshr.length);
    assert(((*length) <= 15) && ((*length) >= 0));
    l2_cache.mshr.mshr_arr[(*length)++] = (MSHR_T) {
        .valid = TRUE,
        .done = FALSE,
        .address = address
    };
}

/*
* Procedure: out_mshr
* Purpose: resolve a memory request out of the queue
*
*/
void out_mshr(uint32_t address) {
    int *length = &(l2_cache.mshr.length);
    assert(((*length) <= 16) && ((*length) > 0));
    int current_pos = -1;
    for (int i = 0; i < (*length); ++i) {
        if ((l2_cache.mshr.mshr_arr[i].valid) && (l2_cache.mshr.mshr_arr[i].address == address)) {
            current_pos = i;
            break;
        }
    }
    assert((current_pos < (*length)) && (current_pos >= 0));
    for (int i = current_pos; i < (*length) - 1; ++i) {
        l2_cache.mshr.mshr_arr[i].address = l2_cache.mshr.mshr_arr[i + 1].address;
        assert(l2_cache.mshr.mshr_arr[i].valid);    
    }
    l2_cache.mshr.mshr_arr[--(*length)].valid = FALSE;
}

/*
* Procedure: probe_mshr
* Purpose: find whether an address is being served by the main memory
*
*/
int probe_mshr(uint32_t address) {
    assert((address % 4 == 0) && (l2_cache.mshr.length <= 16));
    for (int i = 0; i < l2_cache.mshr.length; ++i) {
        if (l2_cache.mshr.mshr_arr[i].valid && (l2_cache.mshr.mshr_arr[i].address == address)) return TRUE;
    }
    return FALSE;
}
