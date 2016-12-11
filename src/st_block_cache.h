/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Wang Jian
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef  _ST_BLOCK_CACHE_H_
#define  _ST_BLOCK_CACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stutils/st_macro.h>

/** @defgroup g_block_cache Block Memory Cache
 * Cache for blocks of memory. Store a series of memory block with fixed size,
 * number of blocks can be extended as needed.
 */

/**
 * block memory cache
 * @ingroup g_block_cache
 */
typedef struct _st_block_cache_t_
{
    void *data; /**< data buffer. */
    int *ref_counts; /**< ref_count for blocks. */
    int block_size; /**< size of block. */
    int count; /**< used count of blocks. */
    int capacity; /**< capacity of blocks. */
    int realloc_count; /**< realloc count of blocks. */

    int *free_blocks; /**< record the ids of free block. */
    int num_free_blocks; /**< number of free blocks. */
} st_block_cache_t;

/**
 * Create a block memory cache.
 * @ingroup g_block_cache
 * @param[in] block_size size of each block.
 * @param[in] init_count initial count of blocks.
 * @param[in] realloc_count count of blocks to realloc when needed,
 *            if non-positive, do not extend.
 * @return block_cache on success, otherwise NULL.
 */
st_block_cache_t* st_block_cache_create(int block_size, int init_count,
        int realloc_count);

/**
 * Destroy a block cache and set the pointer to NULL.
 * @ingroup g_block_cache
 * @param[in] ptr pointer to st_block_cache_t.
 */
#define safe_st_block_cache_destroy(ptr) do {\
    if((ptr) != NULL) {\
        st_block_cache_destroy(ptr);\
        safe_free(ptr);\
        (ptr) = NULL;\
    }\
    } while(0)
/**
 * Destroy a block cache.
 * @ingroup g_block_cache
 * @param[in] bcache cache to be destroyed.
 */
void st_block_cache_destroy(st_block_cache_t* bcache);

/**
 * Get current capacity of block cache
 * @ingroup g_block_cache
 * @param[in] bcache the block cache
 * @return the capacity of block cache, -1 if any error.
 */
int st_block_cache_capacity(st_block_cache_t* bcache);

/**
 * Get current size of block cache
 * @ingroup g_block_cache
 * @param[in] bcache the block cache
 * @return the size of block cache, -1 if any error.
 */
int st_block_cache_size(st_block_cache_t* bcache);

/**
 * Clear all content of a block cache.
 * @ingroup g_block_cache
 * @param[in] bcache the block cache
 * @return non-zero value if any error.
 */
int st_block_cache_clear(st_block_cache_t* bcache);

/**
 * Fetch a block from block cache.
 * if block_id < 0, return a new block form cache, otherwise return the
 * corresponding block.
 * @ingroup g_block_cache
 * @param[in] bcache the block cache
 * @param[in, out] block_id id for the block in cache.
 * @return pointer to the fetched block, NULL if any error.
 */
void* st_block_cache_fetch(st_block_cache_t* bcache, int *block_id);

/**
 * Return a block to block cache.
 * @ingroup g_block_cache
 * @param[in] bcache the block cache
 * @param[in, out] block_id id for the block in cache.
 * @return non-zero if any error.
 */
int st_block_cache_return(st_block_cache_t* bcache, int block_id);

/**
 * Read a block in block cache.
 * @ingroup g_block_cache
 * @param[in] bcache the block cache
 * @param[in, out] block_id id for the block in cache.
 * @return pointer to the fetched block, NULL if any error.
 */
void* st_block_cache_read(st_block_cache_t* bcache, int block_id);

#ifdef __cplusplus
}
#endif

#endif
