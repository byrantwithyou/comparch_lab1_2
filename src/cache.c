/***************************************************************/
/* Yunxin Sun 2020 
/***************************************************************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cache.h"
#include "shell.h"

//TODO:Change the hardcoded shift amount related to A, C, B

/**
Procedure Cache_Init
Purpose: Init the cache
**/
CACHE_T data_cache, ir_cache;
void cache_init(int block_size, int associativity, int cache_size) {
    //====================Initialize the cache paerameter==================
    data_cache.block_size = D_CACHE_B;
    data_cache.associativity = D_CACHE_A;
    data_cache.cache_size = D_CACHE_C;
    ir_cache.block_size = IR_CACHE_B;
    ir_cache.associativity = IR_CACHE_A;
    ir_cache.cache_size = IR_CACHE_C;
    //=====================================================================
    
    //====================Set the valid bit====================
    memset(data_cache.cache_data, 0, sizeof(CACHE_BLOCK_T) * D_CACHE_C / D_CACHE_B);
    memset(ir_cache.cache_data, 0, sizeof(CACHE_BLOCK_T) * IR_CACHE_C / IR_CACHE_B);
    //=====================================================================
    
}

/*
*
* Procedure: Cache_Miss
* Purpose: See if an address misses or hits a certain type of cache 
*/
int cache_miss(uint32_t address, enum CACHE_ENUM cache_type) {
    int i = 0;
    CACHE_T cache_temp = cache_type == DATA_CACHE? data_cache:ir_cache;
    uint32_t tag = cache_type == DATA_CACHE? address >> 13:address >> 11;
    int set = cache_type == DATA_CACHE? address >> 5 & 0xFF:address >> 5 & 0x3F;
    int asso_temp = cache_type == DATA_CACHE? D_CACHE_A:IR_CACHE_A;
    for(; i < asso_temp; ++i) {
        CACHE_BLOCK_T *block_temp = &(cache_temp.cache_data[set][i]);
        if (block_temp->valid && block_temp->tag == tag) return FALSE;
    }
    return TRUE;
}

/*
* Procedure: Cache_Read
* Purpose: Read the data from the given address, assuming that the value hits the cache
*
*/
uint32_t cache_read(uint32_t address, enum CACHE_ENUM cache_type) {
    int i = 0;
    CACHE_T cache_temp = cache_type == DATA_CACHE? data_cache:ir_cache;
    uint32_t tag = cache_type == DATA_CACHE? address >> 13:address >> 11;
    int set = cache_type == DATA_CACHE? address >> 5 & 0xFF:address >> 5 & 0x3F;
    int asso_temp = cache_type == DATA_CACHE? D_CACHE_A:IR_CACHE_A;
    for (; i < asso_temp;++i){
        CACHE_BLOCK_T *block_temp = &(cache_temp.cache_data[set][i]);
        if (block_temp->valid && block_temp->tag == tag) {
            return block_temp->data[address >> 2 & 0x3]; //;word offset in a block
        }
    }
    assert(0);
    return 0x0;   
}

/*
* Procedure: Cache_Write
* Purpose: Write the data to the data cache, assuming that the value hits the cache
*/
void cache_write(uint32_t address, uint32_t data) {

}

/*
* 
* Procedure: Mem_2_Cache
* Purpose: Transfer the data block to cache if cache misses
*/
void mem_2_cache(uint32_t address, enum CACHE_ENUM cache_type);//transfer a block from memory to cache


/*
* Procedure: Cache_2_Mem
* Purpose: Write back a dirty block to memory 
* Note that this only happens when a block is evicted out of the cache, (i.e. this procedure is only called by mem_2_cache)
*/
void cache_2_mem(uint32_t address, enum CACHE_ENUM cache_type) {

}
