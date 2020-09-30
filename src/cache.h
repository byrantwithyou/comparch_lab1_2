/*
 * Computer Architecture
 *
 * Yunxin Sun, 2012
 */

#ifndef _CACHE_H_
#define _CACHE_H_

// Various Cache Parameters
#define D_CACHE_B 32//block size for data cache in byte 
#define D_CACHE_C 0x10000//block size for data cache in byte
#define D_CACHE_A 8//associativity for data cache
#define IR_CACHE_B 32//block size for instruction cache in byte 
#define IR_CACHE_C 0x2000//block size for instruction cache in byte
#define IR_CACHE_A 4//associativity for instruction cache
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MAX_BLOCK_NUM MAX(D_CACHE_C / D_CACHE_B, IR_CACHE_C / IR_CACHE_B) //max block number 
#define MAX_BLOCK_SIZE MAX(D_CACHE_B, IR_CACHE_B) / 4 //max block size in word

/* data structure for a word in a cache*/
typedef struct {
    int valid;
    int dirty;
    int recent;//whether this word is the most recent one in its set;
    int set;//which set it belongs to 
    uint32_t data[MAX_BLOCK_SIZE];//space is allocated based on the size of the block; 
} CACHE_BLOCK_T;

typedef struct {
    int associativity;
    int block_size;//how many byte a block stores;
    int cache_size;//how many byte a cache stores;    
    CACHE_BLOCK_T cache_data[MAX_BLOCK_NUM];//the memory of the cache data is initialized at the cache_init() routine
} CACHE_T;

//define the global cache(data cache and instruction cache)
extern CACHE_T data_cache, ir_cache;

void cache_init();
uint32_t cache_read(uint32_t address);//read from cache
void cache_write(uint32_t address, uint32_t data);//write to cache
void mem_2_cache(uint32_t address);//transfer a block from memory to cache
void cache_2_mem(uint32_t address);//write back a dirty block to memory
#endif
