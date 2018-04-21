#include <stutils/st_macro.h>
#include "st_log.h"
#include "st_mem.h"

#include "st_block_pool.h"

void st_block_pool_destroy(st_block_pool_t *bpool)
{
    if(bpool == NULL) {
        return;
    }

    safe_st_free(bpool->buffer);
    safe_st_free(bpool->free_arr);
}

st_block_pool_t* st_block_pool_create(int capacity, int block_size)
{
    st_block_pool_t* bpool = NULL;

    ST_CHECK_PARAM(capacity <= 0 || block_size <= 0, NULL);

    bpool = (st_block_pool_t *)st_malloc(sizeof(st_block_pool_t));
    if (bpool == NULL) {
        ST_ERROR("Failed to st_malloc bpool");
        goto ERR;
    }

    bpool->buffer = (char *)st_malloc((size_t)capacity * block_size);
    if (bpool->buffer == NULL) {
        ST_ERROR("Failed to st_malloc buffer");
        goto ERR;
    }

    bpool->free_arr = (int *)st_malloc(sizeof(int) * capacity);
    if (bpool->free_arr == NULL) {
        ST_ERROR("Failed to st_malloc free_arr.");
        goto ERR;
    }

    bpool->capacity = capacity;
    bpool->block_size = block_size;

    if (st_block_pool_clear(bpool) < 0) {
        ST_ERROR("Failed to st_block_pool_clear.");
        goto ERR;
    }

    return bpool;

ERR:
    safe_st_block_pool_destroy(bpool);
    return NULL;
}

int st_block_pool_alloc(st_block_pool_t* bpool)
{
    ST_CHECK_PARAM(bpool == NULL, -1);

    if (bpool->free_cur < 0) {
        if(bpool->index_cur >= bpool->capacity) {
            ST_ERROR("block pool overflow");
            return -1;
        }
        return bpool->index_cur++;
    } else {
        return bpool->free_cur--;
    }
}

int st_block_pool_free(st_block_pool_t* bpool, int block_id)
{
    ST_CHECK_PARAM(bpool == NULL || block_id < 0
            || block_id >= bpool->capacity, -1);

    if (bpool->free_cur >= bpool->capacity - 1) {
        ST_ERROR("too many block freed.");
        return -1;
    }

    bpool->free_arr[++bpool->free_cur] = block_id;

    return 0;
}

int st_block_pool_clear(st_block_pool_t* bpool)
{
    ST_CHECK_PARAM(bpool == NULL, -1);

    bpool->index_cur = 0;
    bpool->free_cur  = -1;

    return 0;
}
