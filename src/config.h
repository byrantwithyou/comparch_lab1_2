/*
* cache related config macros
* Yunxin Sun 2020
*/

// implemented policies:
// FIFO, LFU

// implemented insert policies:
// eaf, bip, lip, mru

// implemented replacement policies:
// lru, random

// TODO:
// rrip, arc, dip, victim_cache

#ifndef _CONFIG_H_
#define _CONFIG_H_

    /* =================================Cache Policy Parameter================= */
    // POLICY has a higher priority than the other two policies
    // If policy is defined, the other two does not exist
    // If policy is not defined, use the insert_policy and the replace_policy
    // If nothing is defined, use the default policy 

    #ifndef POLICY
        #define POLICY ""
        #if !(defined(INSERT_POLICY) && defined(REPLACE_POLICY))
            #define INSERT_POLICY "MRU"
            #define REPLACE_POLICY "LRU"
        #endif
    #else
        #define INSERT_POLICY ""
        #define REPLACE_POLICY ""
    #endif

    /* ================================Data Cache Parameter================== */
    
    // associativity for data cache
    #ifndef D_CACHE_A
        #define D_CACHE_A 8            
    #endif
    
    // block size for data cache in byte 
    #ifndef D_CACHE_B
        #define D_CACHE_B 32
    #endif

    // cache size for data cache in byte
    #ifndef D_CACHE_C
        #define D_CACHE_C 0x10000
    #endif

    /* ==================================================================== */

    /* ================================IR Cache Parameter================== */
    
    // associativity for ir cache
    #ifndef IR_CACHE_A
        #define IR_CACHE_A 4            
    #endif

    // block size for ir cache in byte 
    #ifndef IR_CACHE_B
        #define IR_CACHE_B 32
    #endif

    // cache size for ir cache in byte
    #ifndef IR_CACHE_C
        #define IR_CACHE_C 0x2000
    #endif

    /* ==================================================================== */


    /* ============================Utility================================= */
    
    #define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
    
    // max block size in byte
    #define MAX_BLOCK_SIZE MAX(D_CACHE_B, IR_CACHE_B) 
    
    // max set count
    #define MAX_SET_COUNT MAX(D_CACHE_C / (D_CACHE_B * D_CACHE_A), IR_CACHE_C / (IR_CACHE_B * IR_CACHE_A))
    
    // max associativity
    #define MAX_A MAX(D_CACHE_A, IR_CACHE_A)

    /* =================================================================== */

#endif
