#include "policy.h"
// /* =================== replacement ============================== */
//     int lfu = 0, i = 0;
//     int least_frequency = cache->frequency[set][0];
//     for (; i < cache->meta_data.associativity; ++i) {
//         if (cache->frequency[set][i] < least_frequency) {
//             least_frequency = cache->frequency[set][i];
//             lfu = i;
//         }
//     }
//     CACHE_BLOCK_T *evicted_block;
//     if ((strcmp(REPLACE_POLICY, "LRU") == 0) || (strcmp(POLICY, "FIFO") == 0)) {
//         evicted_block = &(cache->data[set][cache->order[set][cache->meta_data.associativity - 1]]);
//     } else if (strcmp(REPLACE_POLICY, "RANDOM") == 0) {
//         evicted_block = &(cache->data[set][rand() % (cache->meta_data.associativity)]);
//     } else if (strcmp(POLICY, "LFU") == 0) {
//         cache->frequency[set][lfu] = 0;
//         evicted_block = &(cache->data[set][lfu]);
//     }
//     if (cache->eaf_length < cache->meta_data.cache_size / cache->meta_data.block_size) {
//         cache->eaf[cache->eaf_length++] = evicted_block->address;
//     } else {
//         assert(cache->eaf_length >= cache->meta_data.cache_size / cache->meta_data.block_size);
//         cache->eaf_length = 0;
//     }
//     return evicted_block;
// /* ============================================================== */

// /* =================================insert======================= */
// if ((strcmp(INSERT_POLICY, "MRU") == 0) \
//     || (strcmp(POLICY, "FIFO") == 0) \
//     || (MRU == TRUE))
//     {
//         int i = 0;
//         int current_order = cache->meta_data.associativity - 1;
//         // find the previous order of the block
//         for (; i < cache->meta_data.associativity - 1; ++i) {
//             if (cache->order[set][i] == most_recently_used) {
//                 current_order = i;
//                 break;
//             }
//             if (cache->order[set][i] == -1) break;
//         }
//         for (i = current_order - 1; i >= 0; --i) {
//             // move others' order
//             cache->order[set][i + 1] = cache->order[set][i];
//         }
//         // set the block to most-recently-used
//         cache->order[set][0] = most_recently_used;
//     } else if (MRU == FALSE) {
//         int current_order = cache->meta_data.associativity - 1;
//         for (int i = 0; i < cache->meta_data.associativity; ++i) {
//             if (cache->order[set][i] == most_recently_used) {
//                 current_order = i;
//                 break;
//             }
//         }
//         for (int i = current_order + 1; i < cache->meta_data.associativity; ++i) {
//             cache->order[set][i - 1] = cache->order[set][i];
//         }
//         cache->order[set][cache->meta_data.associativity - 1] = most_recently_used;
//     }
// /* ============================================================== */


/* ===============================Insert Policy=============================== */
    /* first in, first out */
    void commit_fifo(int way, int set, CACHE_T *cache, int first_commit){

    }
    /* least frequency used */
    void commit_lfu(int way, int set, CACHE_T *cache, int first_commit) {

    }
    /* eaf-cache */
    void commit_eaf(int way, int set, CACHE_T *cache, int first_commit) {

    }
    /* bimodal insertion policy */
    void commit_bip(int way, int set, CACHE_T *cache, int first_commit) {

    }
    /* LRU insertion policy */
    void commit_lip(int way, int set, CACHE_T *cache, int first_commit) {

    }
    /* most recently used */
    void commit_mru(int way, int set, CACHE_T *cache, int first_commit) {

    }
    /* ===============================Replacement Policy========================== */
    // feb stands for find-evicted-block

    CACHE_BLOCK_T *feb_fifo(int set, CACHE_T *cache) {
        return NULL;
    }
    CACHE_BLOCK_T *feb_lfu(int set, CACHE_T *cache) {
        return NULL;
    }
    /* least rencently used */
    CACHE_BLOCK_T *feb_lru(int set, CACHE_T *cache) {
        return NULL;
    }
    /* random replacement */
    CACHE_BLOCK_T *feb_random(int set, CACHE_T *cache) {
        return NULL;
    }
