/*
* Yunxin Sun 2020 
*/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "cache.h"
#include "shell.h"
#include "pipe.h"
#include "policy.h"

//NOTE:assume that the max cache_size is 1GB(according to INT_MAX in <limits.h>), which is reasonable

CACHE_T d_cache, ir_cache;
L2_CACHE_T l2_cache;

//=================Helper Function===============
double log2(double x) {
    assert(x > 0);
    return log(x) / log(2);
} 

/*
* Procudure: Decode_Address
* Purpose: Decode the address to get the cache line meta data 
*/
DECODE_ADDRESS_RESULT_T decode_address(uint32_t address, CACHE_T *cache) {
    int block_size = cache->meta_data.block_size;
    int cache_size = cache->meta_data.cache_size;
    int associativity = cache->meta_data.associativity;
    int set_count = cache_size / (block_size * associativity);
    uint32_t tag = address >> ((int)ceil(log2(block_size)) + (int)ceil(log2(set_count)));
    int set = address >> ((int)ceil(log2(block_size))) & ((1 << (int)ceil(log2(set_count))) - 1);
    return (DECODE_ADDRESS_RESULT_T){.set = set, .tag = tag};
}

/*
* Procudure: Traverse_Block
* Purpose: Generate all the address in a block given the word offset inside a block 
*/
uint32_t traverse_block(uint32_t address, CACHE_T *cache, int word_offset) {
    int offset_bitlen = (int)ceil(log2(cache->meta_data.block_size / 4)) + 2;
    return (address & (~((1 << offset_bitlen) - 1))) + (word_offset << 2);
}

/*
* Procudure: get_offset_in_block
* Purpose: get the word offset inside a cache line 
*/

int get_offset_in_block (uint32_t address, CACHE_T *cache) {
    return address >> 2 & ((1 << (int)ceil(log2(cache->meta_data.block_size / 4))) - 1);
}

/*
* Procudure: Reorder
* Purpose: regenerate the order list according to different policy 
*/
void commit(int way, int set, CACHE_T *cache, int first_commit) {
    ++cache->frequency[set][way];
    if (cache == &(l2_cache.cache) || (strcmp(INSERT_POLICY, "MRU") == 0)) {
        commit_mru(way, set, cache, first_commit);
    } else if (strcmp(INSERT_POLICY, "EAF") == 0) commit_eaf(way, set, cache, first_commit);
    else if (strcmp(INSERT_POLICY, "BIP") == 0) commit_bip(way, set, cache, first_commit);
    else if (strcmp(INSERT_POLICY, "LIP") == 0) commit_lip(way, set, cache, first_commit);
    else if (strcmp(POLICY, "FIFO") == 0) commit_fifo(way, set, cache, first_commit);
    else if (strcmp(POLICY, "LFU") == 0) commit_lfu(way, set, cache, first_commit);
}

/*
* Procudure: Find_Evicted_Block
* Purpose: find the evicted block according to the replacement policy 
*/
CACHE_BLOCK_T *find_evicted_block(int set, CACHE_T *cache) {
    CACHE_BLOCK_T *evicted_block = NULL;
    if ((strcmp(REPLACE_POLICY, "LRU") == 0) || (cache == &(l2_cache.cache))) evicted_block = feb_lru(set, cache);
    else if (strcmp(REPLACE_POLICY, "RANDOM") == 0) evicted_block = feb_random(set, cache);
    else if (strcmp(POLICY, "FIFO") == 0) evicted_block = feb_fifo(set, cache);
    else if (strcmp(POLICY, "LFU") == 0) evicted_block = feb_lfu(set, cache);
    assert(evicted_block);
    if (cache->eaf_length < cache->meta_data.cache_size / cache->meta_data.block_size) {
        cache->eaf[cache->eaf_length++] = evicted_block->meta_data.address;
    } else {
        assert(cache->eaf_length >= cache->meta_data.cache_size / cache->meta_data.block_size);
        cache->eaf_length = 0; 
    }
    return evicted_block;
}

/*
* Procudure: Read_Block_From_Memory
* Purpose: read the whole block from the memory to the cache 
*/
void read_block_from_memory(CACHE_BLOCK_T *block, uint32_t address, CACHE_T *cache, int way) {
    for (int i = 0; i < cache->meta_data.block_size / 4; ++i) {
        block->data[i] = mem_read_32(traverse_block(address, cache, i));}
    block->meta_data = (CACHE_BLOCK_META_T){.valid = TRUE, .dirty = FALSE, .way = way, .address = address};
}
//===============================================

/**
* 
* Procedure Cache_Init
* Purpose: Init the cache
**/
void cache_init() {
    // use for random replacement
    srand(time(NULL));
    //====================Initialize the cache paerameter==================
    d_cache.meta_data = (CACHE_META_T){.associativity = D_CACHE_A, .block_size = D_CACHE_B, .cache_size = D_CACHE_C};
    ir_cache.meta_data = (CACHE_META_T){.associativity = IR_CACHE_A, .block_size = IR_CACHE_B, .cache_size = IR_CACHE_C};
    //=====================================================================
    assert((D_CACHE_C / (D_CACHE_B * D_CACHE_A) >= 1) && (IR_CACHE_C / (IR_CACHE_A * IR_CACHE_B) >= 1));
    //====================Set the valid bit================================
    l2_cache.cache.meta_data = (CACHE_META_T){
        .associativity = 16,
        .block_size = 32,
        .cache_size = 0x20000
    };
    memset(d_cache.data, 0, sizeof d_cache.data);
    memset(ir_cache.data, 0, sizeof ir_cache.data);
    memset(l2_cache.cache.data, 0, sizeof l2_cache.cache.data);
    memset(l2_cache.mshr, 0, sizeof l2_cache.mshr);
    //=====================================================================
    for (int i = 0; i < MAX_SET_COUNT; ++i) {
        for (int j = 0; j < MAX_A; ++j) {
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
    commit(block->meta_data.way, decode_address(address, cache).set, cache, FALSE);
    return block->data[get_offset_in_block(address, cache)];
}

/*
* Procedure: Cache_Write
* Purpose: Write the data to the data cache, assuming that the value hits the cache
*/
void cache_write(uint32_t address, uint32_t data) {
    CACHE_BLOCK_T *block = find_block_position(address, &d_cache);
    assert(block);
    commit(block->meta_data.way, decode_address(address, &d_cache).set, &d_cache, FALSE);
    if (data == block->data[get_offset_in_block(address, &d_cache)]) return;
    block->data[get_offset_in_block(address, &d_cache)] = data;
    block->meta_data.dirty = TRUE;
}

/*
*
* Procedure: Cache_2_Cache
* Purpose: Transfer the data block from cache to cache
*/
void cache2cache(uint32_t address, CACHE_T *cache_dest, CACHE_T *cache_src) {
    assert(cache_dest->meta_data.block_size == cache_src->meta_data.block_size);
    CACHE_BLOCK_T *block_dest = find_block_position(address, cache_dest);
    CACHE_BLOCK_T *block_src = find_block_position(address, cache_src);
    assert(block_dest && block_src && (block_dest->meta_data.address == block_src->meta_data.address));
    block_dest->meta_data.dirty = FALSE;
    block_dest->meta_data.valid = TRUE;
    for (int i = 0; i < MAX_BLOCK_SIZE / 4; ++i) {
        block_dest->data[i] = block_src->data[i];
    }
}

/*
* 
* Procedure: Mem_2_Cache
* Purpose: Transfer the data block to cache if cache misses
*/
void mem2cache(uint32_t address, CACHE_T *cache) {
    int associativity = cache->meta_data.associativity;
    int set = decode_address(address, cache).set;
    for (int i = 0; i < associativity;++i){
        CACHE_BLOCK_T *block = &(cache->data[set][i]);
        if (!block->meta_data.valid) {
            // if find an empty position in the cache, directly insert the block
            read_block_from_memory(block, address, cache, i);
            commit(i, set, cache, TRUE);
            return;
        }
    }
    // otherwise, find an evicted_block
    CACHE_BLOCK_T *evicted_block = find_evicted_block(set, cache);
    if (evicted_block->meta_data.dirty) {
        assert(cache == &d_cache);
        cache2mem(evicted_block->meta_data.address, cache);
        if (find_block_position(evicted_block->meta_data.address, &(l2_cache.cache))) {
            cache2cache(evicted_block->meta_data.address, &(l2_cache.cache), &d_cache);
        }
    }
    read_block_from_memory(evicted_block, address, cache, evicted_block->meta_data.way);
    commit(evicted_block->meta_data.way, set, cache, TRUE);
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
    for (int i = 0; i < cache->meta_data.block_size / 4; ++i) {
        mem_write_32(traverse_block(address, cache, i), block->data[i]);
    }
}

/*
*
* Procedure: Find_Block_Position
* Purpose: Find the pointer to the block according to the address
*/
CACHE_BLOCK_T *find_block_position(uint32_t address, CACHE_T *cache) {
    int associativity = cache->meta_data.associativity;
    int set = decode_address(address, cache).set;
    uint32_t tag = decode_address(address, cache).tag;
    for (int i = 0; i < associativity; ++i) {
        CACHE_BLOCK_T *current_block = &(cache->data[set][i]);
        uint32_t current_tag = decode_address(current_block->meta_data.address, cache).tag;
        if (current_block->meta_data.valid && (current_tag == tag)) {
            return current_block;
        }
    }
    return NULL;
}
