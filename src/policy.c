#include "policy.h"

// /* =================================insert======================= */
// 
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

/* ===============================Helper Function============================ */
int find_current_order(int way, int set, CACHE_T *cache) {
    for (int i = 0; i < cache->meta_data.associativity; ++i) {
        if (cache->order[set][i] == way) return i;
    }
    return -1;
}
/* ========================================================================== */

/* ===============================Insert Policy=============================== */
    /* first in, first out */
    void commit_fifo(int way, int set, CACHE_T *cache, int first_commit){
        if (!first_commit) return;
        commit_mru(way, set, cache, first_commit);
    }
    /* least frequency used */
    void commit_lfu(int way, int set, CACHE_T *cache, int first_commit) {
        return;
    }
    /* eaf-cache */
    void commit_eaf(int way, int set, CACHE_T *cache, int first_commit) {
        if (!first_commit) commit_mru(way, set, cache, first_commit);
        for (int i = 0; i < cache->eaf_length; ++i) {
            if (cache->data[set][way].meta_data.address == cache->eaf[i]){
                commit_mru(way, set, cache, first_commit);
                return;
            }
        }
        commit_bip(way, set, cache, first_commit);
    }
    /* bimodal insertion policy */
    void commit_bip(int way, int set, CACHE_T *cache, int first_commit) {
        if (!first_commit) commit_mru(way, set, cache, first_commit);
        if (rand() % 64 == 0) {
            commit_mru(way, set, cache, first_commit);
        } else {
            commit_lip(way, set, cache, first_commit);
        }
    }
    /* LRU insertion policy */
    void commit_lip(int way, int set, CACHE_T *cache, int first_commit) {
        if (!first_commit) commit_mru(way, set, cache, first_commit);
        int current_order = find_current_order(way, set, cache);
        if (current_order == -1) {
            commit_mru(way, set, cache, first_commit);
            return;
        }
        for (int i = current_order + 1; i < cache->meta_data.associativity; ++i) {
            cache->order[set][i - 1] = cache->order[set][i];
        }
        cache->order[set][cache->meta_data.associativity - 1] = way;
    }
    /* most recently used */
    void commit_mru(int way, int set, CACHE_T *cache, int first_commit) {
        // find previous orders
        int current_order = find_current_order(way, set, cache);
        if (-1 == current_order) current_order = cache->meta_data.associativity - 1;
        for (int i = current_order - 1; i >= 0; --i) {
            // move others' order
            cache->order[set][i + 1] = cache->order[set][i];
        }
        // set the block to most-recently-used
        cache->order[set][0] = way;
    }
    /* ===============================Replacement Policy========================== */
    // feb stands for find-evicted-block

    CACHE_BLOCK_T *feb_fifo(int set, CACHE_T *cache) {
        return feb_lru(set, cache);
    }
    CACHE_BLOCK_T *feb_lfu(int set, CACHE_T *cache) {
        int lfu = 0;
        int least_frequency = cache->frequency[set][0];
        for (int i = 0; i < cache->meta_data.associativity; ++i) {
            if (cache->frequency[set][i] < least_frequency) {
                least_frequency = cache->frequency[set][i];
                lfu = i;
            }
        }
        cache->frequency[set][lfu] = 0;
        return &(cache->data[set][lfu]);
    }
    /* least rencently used */
    CACHE_BLOCK_T *feb_lru(int set, CACHE_T *cache) {
        return &(cache->data[set][cache->order[set][cache->meta_data.associativity - 1]]);
    }
    /* random replacement */
    CACHE_BLOCK_T *feb_random(int set, CACHE_T *cache) {
        return &(cache->data[set][rand() % (cache->meta_data.associativity)]);
    }
