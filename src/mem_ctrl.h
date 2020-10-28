/*
 * Yunxin Sun, 2020
 */


#ifndef _MEM_CTRL_H_
#define _MEM_CTRL_H_
    
    /* Data Structure for memory control */
    typedef struct MSHR_T {
        int valid;
        int done;
        int finished_cycle;
        uint32_t address;
    } MSHR_T;

    typedef struct MSHR_QUEUE {
        int length;
        MSHR_T mshr_arr[16];
    } MSHR_QUEUE;

    typedef struct RANGE_T {
        int min_cycle;
        int max_cycle;
        struct RANGE_T *next;
    } RANGE_T;

    typedef enum ROW_BUFFER_STATUS {
        HIT,
        MISS,
        CONFLICT
    } ROW_BUFFER_STATUS;

    void mem_cycle();
    /* ============Operation for mshr_queue====================== */
    void init_mshr();
    // add an address request into the mshr
    void in_mshr(uint32_t address);
    // when an request is done
    void out_mshr(MSHR_T *request);
    // probe whether an address is in the mshr and is valid
    int probe_mshr(uint32_t address);
    // whether a request is schedulable
    int schedulable(MSHR_T *request);
    // traverse the mshr to find the scheduable and preferred memory request to be served
    // row_hit: whether to consider row_hit or not
    MSHR_T *traverse_mshr(int row_hit);
    // find the request to serve
    MSHR_T *find_request_to_serve();
    // find if there're any requests that have been finished and do clean up
    void finish_request();
    // access the request in the main memory
    void serve_request(MSHR_T *request);
    // set the busy status of the relevant component
    void set_busy_status(enum ROW_BUFFER_STATUS status, int bank);
    void set_bank_busy_status(enum ROW_BUFFER_STATUS status, int bank);
    void set_bus_busy_status(enum ROW_BUFFER_STATUS status, RANGE_T** bus);
    /* ========================================================= */
    /* ===============Helper Function=========================== */
    int get_row(uint32_t address);
    int get_bank(uint32_t address);
    /* ========================================================= */

#endif
