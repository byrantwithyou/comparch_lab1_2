/*
* Yunxin Sun 2020
*/
#include "cache.h"
#include "shell.h"
#include "mem_ctrl.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//TODO:test and optimize

// store which row the 8 bank's row buffer stores, -1 stands for nothing
int row_buffer[8];

// the busy set of three components, this is a linked list
RANGE_T *command_bus_busy_set, *data_bus_busy_set, *bank_busy_set[8];

/* ====================Helper Function=============== */

int intersect(int min_cycle_foo, int min_cycle_bar, int range_length) {
    return abs(min_cycle_foo - min_cycle_bar) <= range_length;
}

void clean_up_helper(RANGE_T **range_list) {
    if ((*range_list) && ((*range_list)->max_cycle < stat_cycles)) {
        assert((*range_list)->max_cycle == stat_cycles - 1);
        RANGE_T *unused_range = *range_list;
        *range_list = (*range_list)->next;
        free(unused_range);
    }
}

void destory(RANGE_T **range_list) {
    while (*range_list) {
        RANGE_T *unused_range = *range_list;
        *range_list = (*range_list)->next;
        free(unused_range);
    }
}

/*
* Procedure: clean_up
* Purpose: clean up unused range
*/
void clean_up() {
    for (int i = 0; i < 8; ++i) {
        clean_up_helper(&(bank_busy_set[i]));
    }
    clean_up_helper(&command_bus_busy_set);
    clean_up_helper(&data_bus_busy_set);
    if (RUN_BIT) {
        destory(&command_bus_busy_set);
        destory(&data_bus_busy_set);
        for (int i = 0; i < 8; ++i) {
            destory(&(bank_busy_set[i]));
        }
    }
}

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
    clean_up();
    MSHR_T *request = find_request_to_serve();
    serve_request(request);
}

/* ================= Operation for mshr_queue ================= */

/*
* Procedure: query_bus_busy_status
* Purpose: check whether the bus is busy
*/
int query_bus_busy_status(RANGE_T **bus_busy_status, int *range_list, \
                        int range_list_length, int range_length) {
    for (int i = 0; i < range_list_length; ++i) {
        RANGE_T *current_iter = *bus_busy_status;
        while (current_iter) {
            if (intersect(range_list[i], current_iter->min_cycle, range_length))
            return TRUE;
            current_iter = current_iter->next;
        }
    }
    return FALSE;
}

/*
* Procedure: set_bus_busy_status
* Purpose: set the bus busy status
*/
void set_bus_busy_status(RANGE_T **bus_busy_status, int *range_list, \
                        int range_list_length, int range_length) {
    RANGE_T *current_iter = *bus_busy_status;
    for (int i = 0; i < range_list_length; ++i) {
        insert(bus_busy_status, range_list[i], range_length);
        merge(bus_busy_status);
    }
}

/*
* Procedure: set_bus_busy_status
* Purpose: set the bus busy status
*/
void merge(RANGE_T **bus_busy_status) {
    RANGE_T *current_iter = *bus_busy_status;
    while (current_iter) {
        if (current_iter->next && (current_iter->max_cycle + 1 == \
        current_iter->next->min_cycle)) {
            RANGE_T *range_temp = current_iter->next;
            current_iter->next = current_iter->next->next;
            free(range_temp);
        }
        current_iter = current_iter->next;
    }
}

/*
*
* Procedure: insert
* Purpose: insert the range
*/
void insert(RANGE_T **bus_busy_status, int range_2_merge, int range_length) {
    RANGE_T *current_iter = *bus_busy_status;
    RANGE_T new_range = (RANGE_T) {
        .min_cycle = stat_cycles,
        .max_cycle = stat_cycles + range_length,
        .next = NULL
    };
    if (!current_iter) {
        *bus_busy_status = malloc(sizeof(RANGE_T));
        **bus_busy_status = new_range;
        return;
    }
    if (current_iter->min_cycle > range_2_merge + range_length) {
        RANGE_T *range_temp = malloc(sizeof(RANGE_T));
        *range_temp = new_range;
        range_temp->next = *bus_busy_status;
        *bus_busy_status = range_temp;
        return;
    }
    while (current_iter) {
        if (!current_iter->next) break;
        if (current_iter->max_cycle < range_2_merge) {
            if (current_iter->next->min_cycle > range_2_merge + range_length) {
                RANGE_T *range_temp = malloc(sizeof(RANGE_T));
                *range_temp = new_range;
                range_temp->next = current_iter->next;
                current_iter->next = range_temp;            
                return;
            }
            current_iter = current_iter->next;
        }
        else {
            assert(FALSE);
        };
    }
    RANGE_T *range_temp = malloc(sizeof(RANGE_T));
    *range_temp = new_range;
    range_temp->next = NULL;
    current_iter->next = range_temp;
}

/*
* Procedure: set_busy_status
* Purpose: set the busy status to revelant components
*/
void set_busy_status(enum ROW_BUFFER_STATUS status, int bank) {
    assert(!bank_busy_set[bank]);
    assert(status >= 0 && status <= 2);
    *(bank_busy_set[bank]) = (RANGE_T) {
        .min_cycle = stat_cycles,
        .max_cycle = stat_cycles + 100 * status,
        .next = NULL
    };
    switch (status) {
        case HIT:
            set_bus_busy_status(&command_bus_busy_set, \
            (int []){stat_cycles}, 1, 4);
            set_bus_busy_status(&data_bus_busy_set, (int []){stat_cycles + 100}, 1, 50);
        case MISS:
            set_bus_busy_status(&command_bus_busy_set, \
            (int []){stat_cycles, stat_cycles + 100}, 2, 4);
            set_bus_busy_status(&data_bus_busy_set, (int []){stat_cycles + 200}, 1, 50);
        case CONFLICT:
            set_bus_busy_status(&command_bus_busy_set, \
            (int []){stat_cycles, stat_cycles + 100, stat_cycles + 300}, 3, 4);
            set_bus_busy_status(&data_bus_busy_set, (int []){stat_cycles + 300}, 1, 50);
        default:
            assert(FALSE);
            break;
    }
}

/*
* Procedure: serve_request()
* Purpose: serve the memory request
*/
void serve_request(MSHR_T *request) {
    if (!request) return;
    uint32_t address = request->address;
    if (row_buffer[get_bank(address)] == -1) {
        request->finished_cycle = stat_cycles + 250;
        row_buffer[get_bank(address)] = get_row(address);
        set_busy_status(MISS, get_bank(address));
    } else if (row_buffer[get_bank(address)] == get_row(address)) {
        request->finished_cycle = stat_cycles + 150;
        set_busy_status(HIT, get_bank(address));
    } else {
        request->finished_cycle = stat_cycles + 350;
        row_buffer[get_bank(address)] = get_row(address);
        set_busy_status(CONFLICT, get_bank(address));
    }
}

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
* Purpose: traverse the mshr to find the schedulable and preferred memory request to be served
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
    command_bus_busy_set = data_bus_busy_set = NULL;
    memset(bank_busy_set, 0, sizeof bank_busy_set);
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
    for (MSHR_T *request_iter = request; \
    request_iter < l2_cache.mshr.mshr_arr + (*length - 1); ++request_iter) {
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
        if (l2_cache.mshr.mshr_arr[i].valid \
        && (l2_cache.mshr.mshr_arr[i].address == address)) 
        return TRUE;
    }
    return FALSE;
}

/* 
*
* Procedure: schedulable
* Purpose: Whether a memory request is schedulable
*/
int schedulable(MSHR_T *request) {
    uint32_t address = request->address;
    int bank = get_bank(address);
    int schedulable = TRUE;
    if (row_buffer[bank] == get_row(address)) {
        if (query_bus_busy_status(&command_bus_busy_set, \
        (int []){stat_cycles}, 1, 4) \
        && query_bus_busy_status(&data_bus_busy_set, \
        (int []){stat_cycles + 100}, 1, 50)) \
        return FALSE;
    }     
    else if (row_buffer[bank] == -1) {
        if (query_bus_busy_status(&command_bus_busy_set, \
        (int []){stat_cycles, stat_cycles + 100}, 2, 4) \
        && query_bus_busy_status(&data_bus_busy_set, \
        (int []){stat_cycles + 200}, 1, 50)) \
        return FALSE;
    } else {
        if (query_bus_busy_status(&command_bus_busy_set, \
        (int []){stat_cycles, stat_cycles + 100, stat_cycles + 200}, 3, 4) \
        && query_bus_busy_status(&data_bus_busy_set, \
        (int []){stat_cycles + 300}, 1, 50)) \
        return FALSE;
    }
    return !bank_busy_set[bank];
}
