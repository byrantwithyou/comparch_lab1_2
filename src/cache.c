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
#include "pipe.h"

//NOTE:assume that the max cache_size is 1GB(according to INT_MAX in <limits.h>), which is reasonable
//TODO:integrete into the pipeline
//test
//TODO:test different parameters and validate

CACHE_T d_cache, ir_cache;

//=================Helper Function===============
double log2(double x) {
    assert(x > 0);
    return log(x) / log(2);
} 

CACHE_BLOCK_META_T decode_address(uint32_t address, CACHE_T *cache) {
    int block_size = cache->meta_data.block_size;
    int cache_size = cache->meta_data.cache_size;
    int associativity = cache->meta_data.associativity;
    int set_count = cache_size / (block_size * associativity);
    uint32_t tag = address >> ((int)ceil(log2(block_size)) + (int)ceil(log2(set_count)));
    int set = address >> ((int)ceil(log2(block_size))) & ((1 << (int)ceil(log2(set_count))) - 1);
    return (CACHE_BLOCK_META_T){.valid = FALSE, .dirty = FALSE, .set = set, .tag = tag, .way = -1};
}

uint32_t encode_address(int set, uint32_t tag, CACHE_T *cache) {
    int block_size = cache->meta_data.block_size;
    int cache_size = cache->meta_data.cache_size;
    int associativity = cache->meta_data.associativity;
    int offset_bitlen = 2 + (int)ceil(log2(block_size / 4));
    return (tag << (offset_bitlen + (int)ceil(log2(cache_size / (associativity * block_size))))) + (set << offset_bitlen);
}

uint32_t traverse_block(uint32_t address, CACHE_T *cache, int word_offset) {
    int offset_bitlen = (int)ceil(log2(cache->meta_data.block_size / 4)) + 2;
    return (address & (~((1 << offset_bitlen) - 1))) + (word_offset << 2);
}

int get_offset_in_block (uint32_t address, CACHE_T *cache) {
    return address >> 2 & ((1 << (int)ceil(log2(cache->meta_data.block_size / 4))) - 1);
}

void reorder(int most_recently_used, int set, CACHE_T *cache) {
    int i = 0;
    int current_order = cache->meta_data.associativity - 1;
    for (; i < cache->meta_data.associativity - 1; ++i) {
        if (cache->order[set][i] == most_recently_used) {
            current_order = i;
            break;
        }
        if (cache->order[set][i] == -1) break;
    }
    for (i = current_order - 1; i >= 0; --i) {
        cache->order[set][i + 1] = cache->order[set][i];
    }
    cache->order[set][0] = most_recently_used;
}

void read_block_from_memory(CACHE_BLOCK_T *block, uint32_t address, CACHE_T *cache, int way) {
    int i = 0;
    for (; i < cache->meta_data.block_size / 4; ++i) {
        block->data[i] = mem_read_32(traverse_block(address, cache, i));}
    int set = decode_address(address, cache).set;
    uint32_t tag = decode_address(address, cache).tag;
    block->meta_data = (CACHE_BLOCK_META_T){.valid = TRUE, .dirty = FALSE, .way = way, .set = set, .tag = tag};
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
    assert((D_CACHE_C / (D_CACHE_B * D_CACHE_A) >= 1) && (IR_CACHE_C / (IR_CACHE_A * IR_CACHE_B) >= 1));
    //====================Set the valid bit================================
    memset(d_cache.data, 0, sizeof d_cache.data);
    memset(ir_cache.data, 0, sizeof ir_cache.data);
    //=====================================================================
    int i = 0, j = 0;
    for (i = 0; i < MAX_SET_COUNT; ++i) {
        for (j = 0; j < MAX_A; ++j) {
            d_cache.order[i][j] = ir_cache.order[i][j] = -1;
        }
    }
}

/*
* Procedure: Cache_Read
* Purpose: Read the data from the given address, assuming that the value hits the cache
* 
*/

uint32_t cache_read(uint32_t address, CACHE_T *cache) {
    CACHE_BLOCK_T *block = find_block_position(address, cache);
    assert(block);
    reorder(block->meta_data.way, decode_address(address, cache).set, cache);
    return block->data[get_offset_in_block(address, cache)];
}

/*
* Procedure: Cache_Write
* Purpose: Write the data to the data cache, assuming that the value hits the cache
*/
void cache_write(uint32_t address, uint32_t data) {
    CACHE_BLOCK_T *block = find_block_position(address, &d_cache);
    assert(block);
    reorder(block->meta_data.way, decode_address(address, &d_cache).set, &d_cache);
    if (data == block->data[get_offset_in_block(address, &d_cache)]) return;
    block->data[get_offset_in_block(address, &d_cache)] = data;
    block->meta_data.dirty = TRUE;
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
        CACHE_BLOCK_T *block = &(cache->data[set][i]);
        if (!block->meta_data.valid) {
            read_block_from_memory(block, address, cache, i);
            reorder(i, set, cache);
            return;
        }
    }
    CACHE_BLOCK_T *evicted_block = &(cache->data[set][cache->order[set][associativity - 1]]);
    if (evicted_block->meta_data.dirty) cache2mem(encode_address(evicted_block->meta_data.set, evicted_block->meta_data.tag, cache), cache);
    read_block_from_memory(evicted_block, address, cache, evicted_block->meta_data.way);
    reorder(evicted_block->meta_data.way, evicted_block->meta_data.set, cache);
}

/*
* Procedure: Cache2Mem
* Purpose: Write back a dirty block to memory 
* Note that this only happens when a block is evicted out of the cache, 
* (i.e. this procedure is only called by mem2cache)
*/
void cache2mem(uint32_t address, CACHE_T *cache) {
    CACHE_BLOCK_T *block = find_block_position(address, cache);
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
        CACHE_BLOCK_T *current_block = &(cache->data[set][i]);
        if (current_block->meta_data.valid && (current_block->meta_data.tag == tag)) {
            return current_block;
        }
    }
    return NULL;
}
