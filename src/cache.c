/***************************************************************/
/* Yunxin Sun 2020 
/***************************************************************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "cache.h"

/**
Procedure Cache_Init
Purpose: Init the cache
**/
void cache_init(int block_size, int associativity, int cache_size) {
    //====================Initialize the cache paerameter==================
    CACHE_T data_cache, ir_cache;
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
