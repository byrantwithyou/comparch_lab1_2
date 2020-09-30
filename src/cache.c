/***************************************************************/
/* Yunxin Sun 2020 
/***************************************************************/
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cache.h"
#include "shell.h"


//TODO:If conflict happens, evict one out, check whether it has dirty bit and write to memory if applicable
//TODO:implement LRU
//order初始化为-1,每当有一个加进来，在那之前的都像后移动
//TODO:static test
//TODO:test
//TODO:integrete
//TODO:test
CACHE_T d_cache, ir_cache;

//=================Helper Function===============
double log2(double x) {
    return log(x) / log(2);
} 
CACHE_BLOCK_META_T decode_address(uint32_t address, CACHE_T *cache) {
    int block_size = cache->meta_data.block_size;
    int cache_size = cache->meta_data.cache_size;
    int associativity = cache->meta_data.associativity;
    int set_count = cache_size / (block_size * associativity);
    uint32_t tag = address >> (2 + (int)ceil(log2(block_size)) + (int)ceil(log2(set_count)));
    int set = address >> (2 + (int)ceil(log2(block_size))) & ((1 << (int)ceil(log2(set_count))) - 1);
    return (CACHE_BLOCK_META_T){.valid = FALSE, .dirty = FALSE, .set = set, .tag = tag};
}
uint32_t traverse_block(uint32_t address, CACHE_T *cache, int word_offset) {
    int offset_bitlen = (int)ceil(log2(cache->meta_data.block_size / 4)) + 2;
    return address & ~(1 << offset_bitlen - 1) + word_offset << 2;
}
int get_offset_in_block (uint32_t address) {
    return address >> 2 & (1 << (int)ceil(log2(d_cache.meta_data.block_size)) - 1);
}
//===============================================

/**
* 
* Procedure Cache_Init
* Purpose: Init the cache
**/
void cache_init() {
    //====================Initialize the cache paerameter==================
    d_cache.meta_data = (CACHE_META_T){.associativity = D_CACHE_A, .block_size = D_CACHE_B, .cache_size = D_CACHE_C};
    ir_cache.meta_data = (CACHE_META_T){.associativity = IR_CACHE_A, .block_size = IR_CACHE_B, .cache_size = IR_CACHE_C};
    //=====================================================================
    
    //====================Set the valid bit====================
    memset(d_cache.cache_data, 0, sizeof(CACHE_BLOCK_T) * D_CACHE_C / D_CACHE_B);
    memset(ir_cache.cache_data, 0, sizeof(CACHE_BLOCK_T) * IR_CACHE_C / IR_CACHE_B);
    //=====================================================================
}

/*
* Procedure: Cache_Read
* Purpose: Read the data from the given address, assuming that the value hits the cache
* 
*/

uint32_t cache_read(uint32_t address, CACHE_T *cache) {
    CACHE_BLOCK_T *block = find_block_position(address, cache);
    assert(block);
    return block->data[get_offset_in_block(address)];
}

/*
* Procedure: Cache_Write
* Purpose: Write the data to the data cache, assuming that the value hits the cache
*/
void cache_write(uint32_t address, uint32_t data) {
    CACHE_BLOCK_T *block = find_block_position(address, &d_cache);
    assert(block);
    block->data[get_offset_in_block(address)] = data;
}

/*
* 
* Procedure: Mem_2_Cache
* Purpose: Transfer the data block to cache if cache misses
*/
void mem2cache(uint32_t address, CACHE_T *cache) {
    int associativity = cache->meta_data.associativity;
    uint32_t tag = decode_address(address, cache).tag;
    int set = decode_address(address, cache).set;
    int i = 0;
    for (; i < associativity;++i){
        CACHE_BLOCK_T *block = &(cache->cache_data[set][i]);
        if (!block->meta_data.valid) {
            block->meta_data = (CACHE_BLOCK_META_T){.valid = TRUE, .dirty = FALSE, .set = set, .tag = tag};
            int j = 0;
            for (; j < cache->meta_data.block_size / 4; ++j) {
                block->data[j] = mem_read_32(traverse_block(address, cache, j));
            }
            return;
        }
    }
    for (; i < associativity;++i) {};

    
}

/*
* Procedure: Cache2Mem
* Purpose: Write back a dirty block to memory 
* Note that this only happens when a block is evicted out of the cache, 
* (i.e. this procedure is only called by mem2cache)
*/
void cache2mem(uint32_t address, CACHE_T *cache) {
    CACHE_BLOCK_T *block = find_block_position(address, cache);
    block->meta_data.valid = FALSE;
    assert(block);
    int i = 0;
    for (; i < cache->meta_data.block_size / 4; ++i) {
        mem_write_32(traverse_block(address, cache, i), block->data[i]);
    }
}

/*
*
* Procedure: Find_Block_Position
* Purpose: Find the pointer to the block according to the address
*/
CACHE_BLOCK_T *find_block_position(uint32_t address, CACHE_T *cache) {
    int i = 0; 
    int associativity = cache->meta_data.associativity;
    int set = decode_address(address, cache).set;
    uint32_t tag = decode_address(address, cache).tag;
    for (; i < associativity; ++i) {
        CACHE_BLOCK_T *current_block = &(cache->cache_data[set][i]);
        if (current_block->meta_data.valid && (current_block->meta_data.tag == tag)) {
            return current_block;
        }
    }
    return NULL;
}
