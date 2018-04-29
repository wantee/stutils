/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Wang Jian
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

#ifndef  _ST_BLOCK_POOL_H_
#define  _ST_BLOCK_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* very simple memory pool for fixed size object.
   The users must avoid double free by themselves.
 */
typedef struct _block_pool_t_ {
    char* buffer;

    int capacity;
    int block_size;

    int index_cur;

    int* free_arr;
    int free_cur;
} st_block_pool_t;

#define st_block_pool_get(bpool, id) ((void *)((bpool)->buffer \
                                         + (size_t)id * (bpool)->block_size))

/**
 * Destroy a block pool and set the pointer to NULL.
 * @ingroup g_block_pool
 * @param[in] ptr pointer to st_block_pool_t.
 */
#define safe_st_block_pool_destroy(ptr) do {\
    if((ptr) != NULL) {\
        st_block_pool_destroy(ptr);\
        safe_st_free(ptr);\
        (ptr) = NULL;\
    }\
    } while(0)
/**
 * Destroy a block pool.
 * @ingroup g_block_pool
 * @param[in] bpool block pool to be destroyed.
 */
void st_block_pool_destroy(st_block_pool_t *bpool);

/**
 * Create a block pool.
 * @ingroup g_block_pool
 * @param[in] capacity number of blocks.
 * @param[in] block_size size of each block.
 * @return block_pool on success, otherwise NULL.
 */
st_block_pool_t* st_block_pool_create(int capacity, int block_size);

/**
 * Clear all content of a block pool.
 * @ingroup g_block_pool
 * @param[in] bpool the block pool
 * @return non-zero value if any error.
 */
int st_block_pool_clear(st_block_pool_t* bpool);

/**
 * Alloc a block from block pool.
 * @ingroup g_block_pool
 * @param[in] bpool the block pool
 * @return index of alloced block, non-zero value if any error.
 */
int st_block_pool_alloc(st_block_pool_t* bpool);

/**
 * Free a block back to block pool.
 * @ingroup g_block_pool
 * @param[in] bpool the block pool.
 * @param[in] block_id index of block to be freed.
 * @return non-zero value if any error.
 */
int st_block_pool_free(st_block_pool_t* bpool, int block_id);

#ifdef __cplusplus
}
#endif
#endif
