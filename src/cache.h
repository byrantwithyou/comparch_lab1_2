/*
 * Yunxin Sun, 2020
 */

#ifndef _CACHE_H_
    #define _CACHE_H_
    #include "config.h"
    #include "stdlib.h"
    #include "stdint.h"
    /* ========================DATA STRUCTURE===================================== */
    
    /* data structure of the meta data of a cache block(line) */
    typedef struct CACHE_BLOCK_META_T{
        int valid; 
        int dirty;
        int way;          //which column the block is in
        uint32_t address; //the address of the cache block
    } CACHE_BLOCK_META_T;

    /* data structure for a block in a cache*/
    typedef struct CACHE_BLOCK_T{
        CACHE_BLOCK_META_T meta_data;
        uint32_t data[MAX_BLOCK_SIZE / 4];//space is allocated based on the size of the block
    } CACHE_BLOCK_T;

    /* data structure of the cache meta data */
    typedef struct CACHE_META_T{
        int associativity;
        int block_size;     //how many byte a block stores
        int cache_size;     //how many byte a cache stores    
    } CACHE_META_T;

    typedef struct CACHE_T{
        CACHE_META_T meta_data;
        /* the memory of the cache data is initialized at the cache_init() routine */
        CACHE_BLOCK_T data[MAX_SET_COUNT][MAX_A];
        /* the order of the block in the set, the former, the ealier */
        int order[MAX_SET_COUNT][MAX_A];  //order of the (recency of) the cache block
        int frequency[MAX_SET_COUNT][MAX_A]; //frequency of the block
        uint32_t eaf[MAX(D_CACHE_C / D_CACHE_B, IR_CACHE_C / IR_CACHE_B)];  // used for eaf insert policy
        int eaf_length; //valide eaf-cache length
    } CACHE_T;

    typedef struct DECODE_ADDRESS_RESULT_T {
        uint32_t tag;
        int set;
    } DECODE_ADDRESS_RESULT_T;

    typedef struct MSHR_T {
        int valid;
        int done;
        uint32_t address;
    } MSHR_T;

    typedef struct L2_CACHE_T {
        CACHE_T cache;
        MSHR_T mshr[16];
    } L2_CACHE_T;

    /* ========================================================================== */

    /* define the global cache(data cache and instruction cache) */
    extern CACHE_T d_cache, ir_cache;
    extern L2_CACHE_T l2_cache;

    void cache_init();
    uint32_t cache_read(uint32_t address, CACHE_T *cache);  //read from cache
    void cache_write(uint32_t address, uint32_t data);      //write to cache
    void mem2cache(uint32_t address, CACHE_T *cache);       //transfer a block from memory to cache
    void cache2mem(uint32_t address, CACHE_T *cache);       //write back a dirty block to memory
    /* find the block address in the cache according to its address */
    CACHE_BLOCK_T *find_block_position(uint32_t address, CACHE_T *cache);
    // when touching a block, commit it, change the data structure accordingly
    // first_commit: whether this is a first-time commit
    void commit(int way, int set, CACHE_T *cache, int first_commit);
    /* read the whole cache line from the memory */
    void read_block_from_memory(CACHE_BLOCK_T *block, uint32_t address, CACHE_T *cache, int way);
    /* find the evicted block according to different policies */
    CACHE_BLOCK_T *find_evicted_block(int set, CACHE_T *cache);

    //=========================Helper Function==========================
    double log2(double x);
    /* decode the address to get the cache line meta data */
    DECODE_ADDRESS_RESULT_T decode_address(uint32_t address, CACHE_T *cache);
    /* generate all the addresses inside one block */
    uint32_t traverse_block(uint32_t address, CACHE_T *cache, int word_offset);
    /* get the word offset inside a cache line */
    int get_offset_in_block(uint32_t address, CACHE_T *cache);
    /* re-generate the order list in the cache for different policies */ 
    //==================================================================

#endif
