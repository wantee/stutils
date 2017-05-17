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

#include <assert.h>
#include <stdio.h>

#include "st_block_cache.h"

#define N 5
static int unit_test_block_cache()
{
    st_block_cache_t *bcache = NULL;
    int ncase = 1;

    int buf[N];
    int i, bid;
    int *data;

    fprintf(stderr, "  Testing st_block_cache...\n");
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    bcache = st_block_cache_create(sizeof(int), N);
    assert(bcache != NULL);
    for (i = 0; i < N; i++) {
        bid = -1;
        data = st_block_cache_fetch(bcache, &bid);
        if (data == NULL) {
          fprintf(stderr, "Failed\n");
          goto ERR;
        }

        *data = i;
        buf[bid] = i;
    }
    fprintf(stderr, "Success\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    for (i = 0; i < N; i++) {
        bid = i;
        data = st_block_cache_fetch(bcache, &bid);
        if (data == NULL) {
          fprintf(stderr, "Failed\n");
          goto ERR;
        }

        if (bid != i) {
          fprintf(stderr, "Failed\n");
          goto ERR;
        }

        if (*data != buf[bid]) {
          fprintf(stderr, "Failed\n");
          goto ERR;
        }
    }
    fprintf(stderr, "Success\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    bid = -1;
    data = st_block_cache_fetch(bcache, &bid);
    if (data == NULL) {
        fprintf(stderr, "Failed\n");
        goto ERR;
    }
    if(st_block_cache_capacity(bcache) != N + N) {
        fprintf(stderr, "Failed\n");
        goto ERR;
    }

    fprintf(stderr, "Success\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    for (i = 0; i < N; i++) {
        bid = i;
        if(st_block_cache_return(bcache, bid) < 0) {
            fprintf(stderr, "Failed\n");
            goto ERR;
        }

        if(st_block_cache_return(bcache, bid) < 0) {
            fprintf(stderr, "Failed\n");
            goto ERR;
        }

        if(st_block_cache_return(bcache, bid) >= 0) {
            fprintf(stderr, "Failed\n");
            goto ERR;
        }
    }
    if(st_block_cache_size(bcache) != 1) {
        fprintf(stderr, "Failed\n");
        goto ERR;
    }
    fprintf(stderr, "Success\n");

    safe_st_block_cache_destroy(bcache);
    return 0;

ERR:
    safe_st_block_cache_destroy(bcache);
    return -1;
}

static int run_all_tests()
{
    int ret = 0;

    if (unit_test_block_cache() != 0) {
        ret = -1;
    }

    return ret;
}

int main(int argc, const char *argv[])
{
    int ret;

    fprintf(stderr, "Start testing...\n");
    ret = run_all_tests();
    if (ret != 0) {
        fprintf(stderr, "Tests failed.\n");
    } else {
        fprintf(stderr, "Tests succeeded.\n");
    }

    return ret;
}
