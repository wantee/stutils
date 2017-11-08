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

#include <string.h>

#include "st_log.h"
#include "st_block_cache.h"

st_block_cache_t* st_block_cache_create(size_t block_size, bcache_id_t count)
{
    st_block_cache_t *bcache = NULL;
    bcache_id_t i;

    ST_CHECK_PARAM(block_size <= 0 || count <= 0, NULL);

    bcache = (st_block_cache_t *)st_malloc(sizeof(st_block_cache_t));
    if (bcache == NULL) {
        ST_ERROR("Failed to st_malloc st_block_cache_t.");
        goto ERR;
    }
    memset(bcache, 0, sizeof(st_block_cache_t));

    bcache->block_size = block_size;
    bcache->count = count;

    bcache->data = (void **)st_malloc(sizeof(void *));
    if (bcache->data == NULL) {
        ST_ERROR("Failed to st_malloc data buffer pool.");
        goto ERR;
    }
    bcache->num_pools = 1;

    bcache->data[0] = (void *)st_malloc(block_size * count);
    if (bcache->data[0] == NULL) {
        ST_ERROR("Failed to st_malloc data buffer.");
        goto ERR;
    }

    bcache->ref_counts = (bcache_id_t *)st_malloc(sizeof(bcache_id_t) * count);
    if (bcache->ref_counts == NULL) {
        ST_ERROR("Failed to st_malloc ref_counts.");
        goto ERR;
    }
    memset(bcache->ref_counts, 0, sizeof(bcache_id_t) * count);

    bcache->free_blocks = (bcache_id_t *)st_malloc(sizeof(bcache_id_t) * count);
    if (bcache->free_blocks == NULL) {
        ST_ERROR("Failed to st_malloc free_blocks.");
        goto ERR;
    }
    for (i = 0; i < count; i++) {
        bcache->free_blocks[i] = i;
    }
    bcache->num_free_blocks = count;

    if (pthread_mutex_init(&bcache->lock, NULL) != 0) {
        ST_ERROR("Failed to pthread_mutex_init lock.");
        goto ERR;
    }

    return bcache;
ERR:
    safe_st_block_cache_destroy(bcache);
    return NULL;
}

void st_block_cache_destroy(st_block_cache_t* bcache)
{
    size_t i;

    if (bcache == NULL) {
        return;
    }

    if (bcache->data != NULL) {
        for (i = 0; i < bcache->num_pools; i++) {
            safe_st_free(bcache->data[i]);
        }
        safe_st_free(bcache->data);
    }
    bcache->num_pools = 0;
    bcache->block_size = 0;
    bcache->count = 0;

    safe_st_free(bcache->ref_counts);
    safe_st_free(bcache->free_blocks);
    bcache->num_free_blocks = 0;

    (void)pthread_mutex_destroy(&bcache->lock);
}

bcache_id_t st_block_cache_capacity(st_block_cache_t* bcache)
{
    return bcache->num_pools * bcache->count;
}

bcache_id_t st_block_cache_size(st_block_cache_t* bcache)
{
    return st_block_cache_capacity(bcache) - bcache->num_free_blocks;
}

int st_block_cache_clear(st_block_cache_t* bcache)
{
    bcache_id_t capacity;
    bcache_id_t i;

    capacity = st_block_cache_capacity(bcache);

    memset(bcache->ref_counts, 0, sizeof(bcache_id_t) * capacity);

    for (i = 0; i < capacity; i++) {
        bcache->free_blocks[i] = i;
    }
    bcache->num_free_blocks = capacity;

    return 0;
}

// should hold the lock outside this function
static bcache_id_t st_bcache_get_free_block(st_block_cache_t *bcache)
{
    bcache_id_t cur_capacity;
    bcache_id_t i;

    if (bcache->num_free_blocks <= 0) {
        cur_capacity = st_block_cache_capacity(bcache);

        bcache->data = (void **)st_realloc(bcache->data, sizeof(void *)
                * (bcache->num_pools + 1));
        if (bcache->data == NULL) {
            ST_ERROR("Failed to st_realloc data.");
            return -1;
        }
        bcache->num_pools += 1;

        bcache->data[bcache->num_pools - 1] = (void *)st_malloc(
                bcache->block_size * bcache->count);
        if (bcache->data[bcache->num_pools - 1] == NULL) {
            ST_ERROR("Failed to st_malloc data buffer.");
            return -1;
        }

        bcache->ref_counts = (bcache_id_t *)st_realloc(bcache->ref_counts,
            sizeof(bcache_id_t) * (cur_capacity + bcache->count));
        if (bcache->ref_counts == NULL) {
            ST_ERROR("Failed to st_realloc ref_counts.");
            return -1;
        }
        memset(bcache->ref_counts + cur_capacity, 0,
                sizeof(bcache_id_t) * bcache->count);

        bcache->free_blocks = (bcache_id_t *)st_realloc(bcache->free_blocks,
            sizeof(bcache_id_t) * (cur_capacity + bcache->count));
        if (bcache->free_blocks == NULL) {
            ST_ERROR("Failed to st_realloc free_blocks.");
            return -1;
        }
        for (i = 0; i < bcache->count; i++) {
            bcache->free_blocks[i] = i + cur_capacity;
        }
        bcache->num_free_blocks = bcache->count;
    }

    return bcache->free_blocks[--bcache->num_free_blocks];
}

// should hold the lock outside this function
static int st_bcache_return_block(st_block_cache_t *bcache,
        bcache_id_t block_id)
{
    bcache->free_blocks[bcache->num_free_blocks++] = block_id;

    return 0;
}

// should hold the lock outside this function
static void* st_bcache_get_block(st_block_cache_t *bcache,
        bcache_id_t block_id)
{
    bcache_id_t p, i;

    p = block_id / bcache->count;
    i = block_id % bcache->count;

    return bcache->data[p] + (bcache->block_size * i);
}

void* st_block_cache_fetch(st_block_cache_t* bcache, bcache_id_t *block_id)
{
    void *ret;

    ST_CHECK_PARAM(bcache == NULL || block_id == NULL
            , NULL);

    if (pthread_mutex_lock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_lock lock.");
        return NULL;
    }
    if (*block_id >= st_block_cache_capacity(bcache)) {
        ST_ERROR("Invalid block_id["BCACHE_ID_FMT".", *block_id);
        goto ERR;
    }

    if (*block_id < 0) {
        *block_id = st_bcache_get_free_block(bcache);
        if (*block_id < 0) {
            ST_ERROR("Failed to st_bcache_get_free_block.");
            goto ERR;
        }
    }

    bcache->ref_counts[*block_id]++;

    ret = st_bcache_get_block(bcache, *block_id);

    if (pthread_mutex_unlock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_unlock lock.");
        return NULL;
    }

    return ret;

ERR:
    if (pthread_mutex_unlock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_unlock lock.");
        return NULL;
    }
    return NULL;
}

int st_block_cache_return(st_block_cache_t* bcache, bcache_id_t block_id)
{
    ST_CHECK_PARAM(bcache == NULL || block_id < 0, -1);

    if (pthread_mutex_lock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_lock lock.");
        return -1;
    }
    if (block_id >= st_block_cache_capacity(bcache)) {
        ST_ERROR("Invalid block_id["BCACHE_ID_FMT".", block_id);
        goto ERR;
    }

    if (bcache->ref_counts[block_id] <= 0) {
        ST_ERROR("block[%d] double returned.", block_id);
        goto ERR;
    }

    bcache->ref_counts[block_id]--;

    if (bcache->ref_counts[block_id] <= 0) {
        if (st_bcache_return_block(bcache, block_id) < 0) {
            ST_ERROR("Failed to st_bcache_return_block.");
            goto ERR;
        }
    }

    if (pthread_mutex_unlock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_unlock lock.");
        return -1;
    }

    return 0;

ERR:
    if (pthread_mutex_unlock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_unlock lock.");
        return -1;
    }
    return -1;
}

void* st_block_cache_read(st_block_cache_t* bcache, bcache_id_t block_id)
{
    void *ret;

    ST_CHECK_PARAM(bcache == NULL || block_id < 0, NULL);

    if (pthread_mutex_lock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_lock lock.");
        return NULL;
    }

    if (block_id >= st_block_cache_capacity(bcache)) {
        ST_ERROR("Invalid block_id["BCACHE_ID_FMT".", block_id);
        goto ERR;
    }

    if (bcache->ref_counts[block_id] <= 0) {
        ST_ERROR("block[%d] not in use.", block_id);
        goto ERR;
    }

    ret = st_bcache_get_block(bcache, block_id);

    if (pthread_mutex_unlock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_unlock lock.");
        return NULL;
    }

    return ret;

ERR:
    if (pthread_mutex_unlock(&bcache->lock) != 0) {
        ST_ERROR("Failed to pthread_mutex_unlock lock.");
        return NULL;
    }

    return NULL;
}
