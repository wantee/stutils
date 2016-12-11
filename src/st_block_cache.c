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

#include "st_log.h"
#include "st_block_cache.h"

st_block_cache_t* st_block_cache_create(int block_size, int init_count,
        int realloc_count)
{
    st_block_cache_t *bcache = NULL;
    int i;

    ST_CHECK_PARAM(block_size <= 0 || init_count <= 0, NULL);

    bcache = (st_block_cache_t *)malloc(sizeof(st_block_cache_t));
    if (bcache == NULL) {
        ST_WARNING("Failed to malloc st_block_cache_t.");
        goto ERR;
    }
    memset(bcache, 0, sizeof(st_block_cache_t));

    bcache->block_size = block_size;
    bcache->capacity = init_count;
    bcache->realloc_count = realloc_count;

    bcache->data = malloc(block_size * init_count);
    if (bcache->data == NULL) {
        ST_WARNING("Failed to malloc data.");
        goto ERR;
    }

    bcache->ref_counts = (int *)malloc(sizeof(int) * init_count);
    if (bcache->ref_counts == NULL) {
        ST_WARNING("Failed to malloc ref_counts.");
        goto ERR;
    }
    memset(bcache->ref_counts, 0, sizeof(int) * init_count);

    bcache->free_blocks = (int *)malloc(sizeof(int) * init_count);
    if (bcache->free_blocks == NULL) {
        ST_WARNING("Failed to realloc free_blocks.");
        goto ERR;
    }
    for (i = 0; i < init_count; i++) {
        bcache->free_blocks[i] = i;
    }
    bcache->num_free_blocks = init_count;

    return bcache;
ERR:
    safe_st_block_cache_destroy(bcache);
    return NULL;
}

void st_block_cache_destroy(st_block_cache_t* bcache)
{
    if (bcache == NULL) {
        return;
    }

    safe_free(bcache->data);
    safe_free(bcache->ref_counts);

    bcache->capacity = 0;
    bcache->block_size = 0;
    bcache->realloc_count = 0;

    safe_free(bcache->free_blocks);
    bcache->num_free_blocks = 0;
}

int st_block_cache_capacity(st_block_cache_t* bcache)
{
    return bcache->capacity;
}

int st_block_cache_size(st_block_cache_t* bcache)
{
    return bcache->capacity - bcache->num_free_blocks;
}

int st_block_cache_clear(st_block_cache_t* bcache)
{
    int i;

    memset(bcache->ref_counts, 0, sizeof(int) * bcache->capacity);

    for (i = 0; i < bcache->capacity; i++) {
        bcache->free_blocks[i] = i;
    }
    bcache->num_free_blocks = bcache->capacity;

    return 0;
}

static int st_bcache_get_free_block(st_block_cache_t *bcache)
{
    int i;

    if (bcache->num_free_blocks <= 0) {
        if (bcache->realloc_count <= 0) {
            ST_WARNING("block cache overflow.");
            return -1;
        }

        bcache->data = realloc(bcache->data, bcache->block_size
                * (bcache->capacity + bcache->realloc_count));
        if (bcache->data == NULL) {
            ST_WARNING("Failed to realloc data.");
            return -1;
        }

        bcache->ref_counts = (int *)realloc(bcache->ref_counts,
                sizeof(int) * (bcache->capacity + bcache->realloc_count));
        if (bcache->ref_counts == NULL) {
            ST_WARNING("Failed to realloc ref_counts.");
            return -1;
        }
        memset(bcache->ref_counts + bcache->capacity, 0,
                sizeof(int) * bcache->realloc_count);

        bcache->free_blocks = (int *)realloc(bcache->free_blocks,
                sizeof(int) * (bcache->capacity + bcache->realloc_count));
        if (bcache->free_blocks == NULL) {
            ST_WARNING("Failed to realloc free_blocks.");
            return -1;
        }
        for (i = 0; i < bcache->realloc_count; i++) {
            bcache->free_blocks[i] = i + bcache->capacity;
        }
        bcache->num_free_blocks = bcache->realloc_count;
        bcache->capacity += bcache->realloc_count;
    }

    return bcache->free_blocks[--bcache->num_free_blocks];
}

static int st_bcache_return_block(st_block_cache_t *bcache, int block_id)
{
    bcache->free_blocks[bcache->num_free_blocks++] = block_id;

    return 0;
}

void* st_block_cache_fetch(st_block_cache_t* bcache, int *block_id)
{
    ST_CHECK_PARAM(bcache == NULL || block_id == NULL
            || *block_id >= bcache->capacity, NULL);

    if (*block_id < 0) {
        *block_id = st_bcache_get_free_block(bcache);
        if (*block_id < 0) {
            ST_WARNING("Failed to st_bcache_get_free_block.");
            return NULL;
        }
    }

    bcache->ref_counts[*block_id]++;

    return bcache->data + (bcache->block_size * (*block_id));
}

int st_block_cache_return(st_block_cache_t* bcache, int block_id)
{
    ST_CHECK_PARAM(bcache == NULL || block_id < 0
            || block_id >= bcache->capacity, -1);

    if (bcache->ref_counts[block_id] <= 0) {
        ST_WARNING("block[%d] double returned.", block_id);
        return -1;
    }

    bcache->ref_counts[block_id]--;

    if (bcache->ref_counts[block_id] <= 0) {
        if (st_bcache_return_block(bcache, block_id) < 0) {
            ST_WARNING("Failed to st_bcache_return_block.");
            return -1;
        }
    }

    return 0;
}

void* st_block_cache_read(st_block_cache_t* bcache, int block_id)
{
    ST_CHECK_PARAM(bcache == NULL || block_id < 0
            || block_id >= bcache->capacity, NULL);

    if (bcache->ref_counts[block_id] <= 0) {
        ST_WARNING("block[%d] not in use.", block_id);
        return NULL;
    }

    return bcache->data + bcache->block_size * block_id;
}
