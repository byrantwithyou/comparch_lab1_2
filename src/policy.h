/*
* Implementation of different policies
* Yunxin Sun 2020
*/
#ifndef _POLICY_H_
    #define _POLICY_H_

    #include "cache.h"
    #include "stdint.h"
    #include "stdlib.h"
    /* ===============================Insert Policy=============================== */
    /* first in, first out */
    void commit_fifo(int way, int set, CACHE_T *cache, int first_commit);
    /* least frequency used */
    void commit_lfu(int way, int set, CACHE_T *cache, int first_commit);
    /* eaf-cache[1] */
    void commit_eaf(int way, int set, CACHE_T *cache, int first_commit);
    /* bimodal insertion policy[2] */
    void commit_bip(int way, int set, CACHE_T *cache, int first_commit);
    /* LRU insertion policy[2] */
    void commit_lip(int way, int set, CACHE_T *cache, int first_commit);
    /* most recently used */
    void commit_mru(int way, int set, CACHE_T *cache, int first_commit);
    /* ===============================Replacement Policy========================== */
    // feb stands for find-evicted-block

    CACHE_BLOCK_T *feb_fifo(int set, CACHE_T *cache);
    CACHE_BLOCK_T *feb_lfu(int set, CACHE_T *cache);
    /* least rencently used */
    CACHE_BLOCK_T *feb_lru(int set, CACHE_T *cache);
    /* random replacement */
    CACHE_BLOCK_T *feb_random(int set, CACHE_T *cache);
#endif

// [1] Seshadri, Vivek, et al. "The evicted-address filter: A unified mechanism to address both cache pollution and thrashing." 2012 21st International Conference on Parallel Architectures and Compilation Techniques (PACT). IEEE, 2012.
// [2] Qureshi, Moinuddin K., et al. "Adaptive insertion policies for high performance caching." ACM SIGARCH Computer Architecture News 35.2 (2007): 381-391.
