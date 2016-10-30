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

#include <stdio.h>

#include "st_queue.h"

static int unit_test_queue()
{
    st_queue_t *queue = NULL;
    st_queue_id_t queue_size = 3;
    void *tmp;
    int i;
    int ncase;

    fprintf(stderr, " Testing st_queue...\n");

    queue = st_queue_create(queue_size);

    ncase = 1;
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    st_queue_clear(queue);
    if (!st_queue_empty(queue)) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    for (i = 0; i < queue_size; i++) {
        if (st_enqueue(queue, (void *)(long)i) != ST_QUEUE_OK) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }
    if (st_enqueue(queue, (void *)(long)i) != ST_QUEUE_FULL) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    for (i = 0; i < queue_size; i++) {
        if (st_dequeue(queue, &tmp) != ST_QUEUE_OK) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
        if ((int)(long)tmp != i) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }
    if (st_dequeue(queue, &tmp) != ST_QUEUE_EMPTY) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }

    for (i = 0; i < queue_size - 1; i++) {
        if (st_enqueue(queue, (void *)(long)i) != ST_QUEUE_OK) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }
    for (i = 0; i < queue_size - 1; i++) {
        if (st_dequeue(queue, &tmp) != ST_QUEUE_OK) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
        if ((int)(long)tmp != i) {
            fprintf(stderr, "Failed\n");
            goto FAILED;
        }
    }
    fprintf(stderr, "Passed\n");

    safe_st_queue_destroy(queue);
    return 0;

FAILED:
    safe_st_queue_destroy(queue);
    return -1;
}

static int run_all_tests()
{
    int ret = 0;

    if (unit_test_queue() != 0) {
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
