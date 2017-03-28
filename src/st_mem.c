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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <stutils/st_macro.h>
#include "st_log.h"
#include "st_mem.h"

#define is_power_of_two(x) (((x) != 0) && !((x) & ((x) - 1)))

typedef struct _st_mem_usage_t_ {
    pthread_mutex_t lock;
    size_t size;
    size_t peak;
    size_t num_allocs;
    size_t num_frees;
} st_mem_usage_t;

st_mem_usage_t g_usage;
bool g_collect_usage = false;

int st_mem_usage_init()
{
    if (pthread_mutex_init(&g_usage.lock, NULL) != 0) {
        ST_WARNING("Failed to pthread_mutex_init lock.");
        return -1;
    }

    g_collect_usage = true;

    return 0;
}

void st_mem_usage_report()
{
    if (! g_collect_usage) {
        return;
    }

    ST_CLEAN("Memory Usage:");
    ST_CLEAN("Peak: %zu", g_usage.peak);
    ST_CLEAN("#allocs: %zu", g_usage.num_allocs);
    ST_CLEAN("#frees: %zu", g_usage.num_frees);
}

void st_mem_usage_destroy()
{
    (void) pthread_mutex_destroy(&g_usage.lock);
    g_usage.peak = 0;
    g_usage.size = 0;
    g_usage.num_allocs = 0;
    g_usage.num_frees = 0;

    g_collect_usage = false;
}

void* st_malloc_impl(size_t size)
{
    size_t *p;

    if (! g_collect_usage) {
        return malloc(size);
    }

    p = (size_t *)malloc(size + sizeof(size_t));
    if (p == NULL) {
        ST_WARNING("Failed to malloc[%zu].", size + sizeof(size_t));
        return NULL;
    }

    p[0] = size; // store size

    if (pthread_mutex_lock(&g_usage.lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_lock.");
        return NULL;
    }

    g_usage.size += size;
    if (g_usage.size > g_usage.peak) {
        g_usage.peak = g_usage.size;
    }
    g_usage.num_allocs++;

    if (pthread_mutex_unlock(&g_usage.lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_unlock.");
        return NULL;
    }

    return (void *)(p + 1);
}

void* st_realloc_impl(void *ptr, size_t size)
{
    size_t *p;
    size_t old_size;

    if (! g_collect_usage) {
        return realloc(ptr, size);
    }

    p = (size_t *)ptr;
    if (p == NULL) {
        old_size = 0;
    } else {
        --p; // recover starting point
        old_size = p[0];
    }

    p = (size_t *)realloc(p, size + sizeof(size_t));
    if (p == NULL) {
        ST_WARNING("Failed to realloc[%zu].", size + sizeof(size_t));
        return NULL;
    }

    p[0] = size; // store size

    if (pthread_mutex_lock(&g_usage.lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_lock.");
        return NULL;
    }

    g_usage.size += size - old_size;
    if (g_usage.size > g_usage.peak) {
        g_usage.peak = g_usage.size;
    }
    g_usage.num_allocs++;
    g_usage.num_frees++;

    if (pthread_mutex_unlock(&g_usage.lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_unlock.");
        return NULL;
    }

    return (void *)(p + 1);
}

void st_free(void *p)
{
    size_t *p1;
    size_t size;

    if (! g_collect_usage) {
        free(p);
        return;
    }

    p1 = (size_t *)p;
    size = p1[-1];

    free(p1 - 1);

    if (pthread_mutex_lock(&g_usage.lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_lock.");
        return;
    }

    g_usage.size -= size;
    g_usage.num_frees++;

    if (pthread_mutex_unlock(&g_usage.lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_unlock.");
        return;
    }
}

size_t st_mem_size(void *p)
{
    size_t *p1;

    if (p == NULL) {
        return 0;
    }

    p1 = (size_t *)p;
    return p1[-1];
}

void* st_aligned_malloc_impl(size_t size, size_t alignment)
{
    void *p1; // original block
    void *p2; // aligned block
    size_t *p3;
    size_t padding;

    if (!is_power_of_two(alignment)) {
        ST_WARNING("alignment[%zu] is not power of 2.", alignment);
        return NULL;
    }

    padding = alignment - 1 + 3 * sizeof(size_t);
    p1 = (void *)malloc(size + padding);
    if (p1 == NULL) {
        ST_WARNING("Failed to malloc[%zu].", size + padding);
        return NULL;
    }
    p2 = (void *)(((size_t)p1 + padding) & ~(alignment - 1)); // insert padding
    p3 = (size_t *)p2;
    p3[-1] = (char *)p2 - (char *)p1; // store offset
    p3[-2] = alignment; // store alignment
    p3[-3] = size; // store size

    if (g_collect_usage) {
        if (pthread_mutex_lock(&g_usage.lock) != 0) {
            ST_WARNING("Failed to pthread_mutex_lock.");
            return NULL;
        }

        g_usage.size += size;
        if (g_usage.size > g_usage.peak) {
            g_usage.peak = g_usage.size;
        }
        g_usage.num_allocs++;

        if (pthread_mutex_unlock(&g_usage.lock) != 0) {
            ST_WARNING("Failed to pthread_mutex_unlock.");
            return NULL;
        }
    }

    return p2;
}

void* st_aligned_realloc_impl(void *ptr, size_t size, size_t alignment)
{
    void *p1, *q1; // original block
    void *q2; // aligned block
    size_t *p3;
    size_t ori_alignment, ori_offset, ori_size = 0;
    size_t padding;

    if (!is_power_of_two(alignment)) {
        ST_WARNING("alignment[%zu] is not power of 2.", alignment);
        return NULL;
    }

    if (ptr == NULL) {
        q2 = st_aligned_malloc(size, alignment);
        if (q2 == NULL) {
            ST_WARNING("Failed to st_aligned_malloc.");
            return NULL;
        }

        goto RET;
    }

    p3 = (size_t *)ptr;
    ori_offset = p3[-1];
    ori_alignment = p3[-2];
    ori_size = p3[-3];

    p1 = (char *)ptr - ori_offset;

    padding = alignment - 1 + 3 * sizeof(size_t);
    q1 = (void *)realloc(p1, size + padding);
    if (q1 == NULL) {
        ST_WARNING("Failed to realloc[%zu].", size + padding);
        q2 = NULL;
        goto RET;
    }

    if (alignment != ori_alignment) {
        goto REALIGN;
    }

    if (p1 == q1) { /* block is expanded in-place. */
        p3 = (size_t *)ptr;
        p3[-3] = size;

        q2 = ptr;
        goto RET;
    }

    q2 = q1 + ori_offset;
    if (((size_t)q2 & (alignment - 1)) == 0) {
        /* realloc happens to give us a  correctly aligned block. */
        p3 = (size_t *)q2;
        p3[-3] = size;

        goto RET;
    }

REALIGN:
    /* realign the block. */
    q2 = (void *)(((size_t)q1 + padding) & ~(alignment - 1)); // insert padding
    memmove(q2, q1 + ori_offset, ori_size);
    p3 = (size_t *)q2;
    p3[-1] = (char *)q2 - (char *)q1; // store offset
    p3[-2] = alignment; // store alignment
    p3[-3] = size; // store size

RET:
    if (g_collect_usage) {
        if (pthread_mutex_lock(&g_usage.lock) != 0) {
            ST_WARNING("Failed to pthread_mutex_lock.");
            return NULL;
        }

        g_usage.size += size - ori_size;
        if (g_usage.size > g_usage.peak) {
            g_usage.peak = g_usage.size;
        }
        g_usage.num_frees++;
        g_usage.num_allocs++;

        if (pthread_mutex_unlock(&g_usage.lock) != 0) {
            ST_WARNING("Failed to pthread_mutex_unlock.");
            return NULL;
        }
    }

    return q2;
}

void st_aligned_free(void *p)
{
    void *p1;
    size_t *p3;
    size_t size;

    p3 = (size_t *)p;
    p1 = (char *)p - p3[-1];
    size = p3[-3];

    free(p1);

    if (g_collect_usage) {
        if (pthread_mutex_lock(&g_usage.lock) != 0) {
            ST_WARNING("Failed to pthread_mutex_lock.");
            return;
        }

        g_usage.size -= size;
        g_usage.num_frees++;

        if (pthread_mutex_unlock(&g_usage.lock) != 0) {
            ST_WARNING("Failed to pthread_mutex_unlock.");
            return;
        }
    }
}

size_t st_aligned_alignment(void *p)
{
    size_t *p3;

    if (p == NULL) {
        return 0;
    }

    p3 = (size_t *)p;
    return p3[-2];
}

size_t st_aligned_size(void *p)
{
    size_t *p3;

    if (p == NULL) {
        return 0;
    }

    p3 = (size_t *)p;
    return p3[-3];
}

#ifdef _ST_MEM_DEBUG_
void* st_malloc_wrapper(size_t size, const char *file, size_t line,
        const char *func)
{
    ST_CLEAN("[%s:%zu<<%s>>] st_malloc: %zu", file, line, func, size);
    return st_malloc_impl(size);
}

void* st_realloc_wrapper(void *p, size_t size, const char *file, size_t line,
        const char *func)
{
    if (g_collect_usage) {
        ST_CLEAN("[%s:%zu<<%s>>] st_realloc: %zu - %zu = %zu", file, line, func,
                size, st_mem_size(p), size - st_mem_size(p));
    } else {
        ST_CLEAN("[%s:%zu<<%s>>] st_realloc: %zu", file, line, func, size);
    }

    return st_realloc_impl(p, size);
}

void* st_aligned_malloc_wrapper(size_t size, size_t alignment,
        const char *file, size_t line, const char *func)
{
    ST_CLEAN("[%s:%zu<<%s>>] st_aligned_malloc: %zu", file, line, func, size);
    return st_aligned_malloc_impl(size, alignment);
}

void* st_aligned_realloc_wrapper(void *p, size_t size, size_t alignment,
        const char *file, size_t line, const char *func)
{
    ST_CLEAN("[%s:%zu<<%s>>] st_aligned_realloc: %zu - %zu = %zu", file, line, func,
            size, st_aligned_size(p), size - st_aligned_size(p));
    return st_aligned_realloc_impl(p, size, alignment);
}
#endif // _ST_MEM_DEBUG_
