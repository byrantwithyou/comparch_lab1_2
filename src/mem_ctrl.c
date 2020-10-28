/*
* Yunxin Sun 2020
*/
#include "cache.h"
#include "shell.h"
#include <string.h>
#include <assert.h>


//TODO:serve_request function:set the status to busy, calculate finished_cycle
//TODO:prune working set status
//TODO:schedulable function
//TODO:test

// store which row the 8 bank's row buffer stores, -1 stands for nothing
int row_buffer[8];

/* ====================Helper Function=============== */

/* 
* 
* Procedure: get_row
* Purpose: get the row id from address
*
*/
int get_row(uint32_t address) {
    return address >> 16;
}

/* 
* 
* Procedure: get_bank
* Purpose: get the bank id from address
*
*/
int get_bank(uint32_t address) {
    return address >> 5 & 0x7;
}

/* ================================================== */
/*
* Procedure: mem_cycle()
* Purpose: main memory cycle
*/
void mem_cycle() {
    finish_request();
    MSHR_T *request = find_request_to_serve();
    // serve request
}

/* ================= Operation for mshr_queue ================= */

/*
* Procedure: finish_request()
* Purpose: find if there're any requests that have been finished and do clean up
*/
void finish_request() {
    for (int i = 0; i < l2_cache.mshr.length; ++i) {
        if (l2_cache.mshr.mshr_arr[i].finished_cycle == stat_cycles) {
            out_mshr(l2_cache.mshr.mshr_arr + i);
        }
    }
}

/*
* Procedure: traverse_msher
* Purpose: traverse the mshr to find the scheduable and preferred memory request to be served
*/
MSHR_T *traverse_mshr(int row_hit) {
    for (int i = 0; i < l2_cache.mshr.length; ++i) {
        assert(l2_cache.mshr.mshr_arr[i].valid);
        MSHR_T *request = l2_cache.mshr.mshr_arr + i;
        uint32_t address = request->address;
        int same_row = (get_row(address) == row_buffer[get_bank(address)]);
        if (schedulable(request) && (!row_hit || same_row)) {
            return l2_cache.mshr.mshr_arr + i;
        }
    }
    return NULL;
}

/* 
* Procedure: find_request_to_serve()
* Purpose: find the request in the queue to serve
*/
MSHR_T *find_request_to_serve() {
    MSHR_T *request = traverse_mshr(TRUE);
    if (!request) request = traverse_mshr(FALSE);
    return request;
}

/*
* Procedure: init_mshr()
* Purpose: initialize the mshr queue
*
*/
void init_mshr() {
    l2_cache.mshr.length = 0;
    memset(l2_cache.mshr.mshr_arr, 0, sizeof l2_cache.mshr.mshr_arr);
    for (int i = 0; i < 8; ++i) {
        row_buffer[i] = -1;
    }
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
        .address = address,
        .finished_cycle = 0
    };
}

/*
* Procedure: out_mshr
* Purpose: resolve a memory request out of the queue
*
*/
void out_mshr(MSHR_T *request) {
    int *length = &(l2_cache.mshr.length);
    assert(((*length) <= 16) && ((*length) > 0) && request);
    for (MSHR_T *request_iter = request; request_iter < l2_cache.mshr.mshr_arr + (*length - 1); ++request_iter) {
        request->address = (request + 1)->address;
        request->finished_cycle = (request + 1)->finished_cycle;
        assert(request_iter->valid);
    }
    --(*length);
    (l2_cache.mshr.mshr_arr + *length) -> valid = FALSE;
    (l2_cache.mshr.mshr_arr + *length) -> finished_cycle = 0;
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

/* 
*
* Procedure: schedulable
* Purpose: Whether a memory request is schedulable
*/
int schedulable(MSHR_T *request) {
    return TRUE;
}
