/*
 * Computer Architecture
 *
 * Yunxin Sun, 2012
 */

#ifndef _CACHE_H_
#define _CACHE_H_

/* various cache parameters */
#define D_CACHE_B 32           //block size for data cache in byte 
#define D_CACHE_C 0x10000      //cache size for data cache in byte
#define D_CACHE_A 8            //associativity for data cache

#define IR_CACHE_B 32          //block size for instruction cache in byte 
#define IR_CACHE_C 0x2000      //cache size for instruction cache in byte
#define IR_CACHE_A 4           //associativity for instruction cache

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
/* max block size in byte */
#define MAX_BLOCK_SIZE MAX(D_CACHE_B, IR_CACHE_B) 
/* max set number */
#define MAX_SET_COUNT MAX(D_CACHE_C / (D_CACHE_B * D_CACHE_A), IR_CACHE_C / (IR_CACHE_B * IR_CACHE_A))
/* max associativity */
#define MAX_A MAX(D_CACHE_A, IR_CACHE_A)

/* The Policy for insert and replacement */
#define INSERT_POLICY "MRU"
#define REPLACE_POLICY "RANDOM"

/* For some, it includes both insert and replacment policy */
#define POLICY ""

/* data structure of the meta data of a cache block(line) */
typedef struct {
    int valid; 
    int dirty;
    int way;       //which column the block is in
    int set;       //which set the block belongs to
    uint32_t tag;  //the tag of the block, assuming that the LSBs are all 0
} CACHE_BLOCK_META_T;

/* data structure for a block in a cache*/
typedef struct {
    CACHE_BLOCK_META_T meta_data;
    uint32_t data[MAX_BLOCK_SIZE / 4];//space is allocated based on the size of the block
} CACHE_BLOCK_T;

/* data structure of the cache meta data */
typedef struct {
    int associativity;
    int block_size;     //how many byte a block stores
    int cache_size;     //how many byte a cache stores    
} CACHE_META_T;

typedef struct {
    CACHE_META_T meta_data;
    /* the memory of the cache data is initialized at the cache_init() routine */
    CACHE_BLOCK_T data[MAX_SET_COUNT][MAX_A];
    /* the order of the block in the set, the former, the ealier */
    int order[MAX_SET_COUNT][MAX_A];
} CACHE_T;

/* define the global cache(data cache and instruction cache) */
extern CACHE_T d_cache, ir_cache;


void cache_init();
uint32_t cache_read(uint32_t address, CACHE_T *cache);  //read from cache
void cache_write(uint32_t address, uint32_t data);      //write to cache
void mem2cache(uint32_t address, CACHE_T *cache);       //transfer a block from memory to cache
void cache2mem(uint32_t address, CACHE_T *cache);       //write back a dirty block to memory
/* find the block address in the cache according to its address */
CACHE_BLOCK_T *find_block_position(uint32_t address, CACHE_T *cache);
//=========================Helper Function==========================
double log2(double x);
/* decode the address to get the cache line meta data */
CACHE_BLOCK_META_T decode_address(uint32_t address, CACHE_T *cache);
/* encode the address given the set and the tag */
uint32_t encode_address(int set, uint32_t tag, CACHE_T *cache);
/* generate all the addresses inside one block */
uint32_t traverse_block(uint32_t address, CACHE_T *cache, int word_offset);
/* get the word offset inside a cache line */
int get_offset_in_block(uint32_t address, CACHE_T *cache);
/* re-generate the order list in the cache for different policies */ 
void reorder(int most_recently_used, int set, CACHE_T *cache);
/* read the whole cache line from the memory */
void read_block_from_memory(CACHE_BLOCK_T *block, uint32_t address, CACHE_T *cache, int way);
/* find the evicted block according to different policies */
CACHE_BLOCK_T *find_evicted_block(int set, CACHE_T *cache);
//==================================================================
#endif
