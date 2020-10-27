/*
 * Yunxin Sun, 2020
 */


#ifndef _MEM_CTRL_H_
#define _MEM_CTRL_H_

    /* Data Structure for MSHR, this is a queue */
    typedef struct MSHR_T {
        int valid;
        int done;
        uint32_t address;
    } MSHR_T;

    typedef struct MSHR_QUEUE {
        int length;
        MSHR_T mshr_arr[16];
    } MSHR_QUEUE;

    void mem_cycle();
    /* Operation for mshr_queue */
    void init_mshr();
    // add an address request into the mshr
    void in_mshr(uint32_t address);
    // when an request is done
    void out_mshr(uint32_t address);
    // probe whether an address is in the mshr and is valid
    int probe_mshr(uint32_t address);
#endif
