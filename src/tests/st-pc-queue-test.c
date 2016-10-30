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
#include <assert.h>
#include <pthread.h>

#include "st_rand.h"

#include "st_pc_queue.h"

static bool diff_fp(FILE *fp1, FILE *fp2)
{
    int a;
    int b;

    do {
        a = fgetc(fp1);
        b = fgetc(fp2);

        if (a != b) {
            return false;
        }
    } while (a != EOF);

    return true;
}

static FILE *enqueue_fp = NULL;
static FILE *dequeue_fp = NULL;

int enqueue_callback(st_pc_queue_t *queue, void *obj)
{
    return fprintf(enqueue_fp, "%d\n", (int)(long)obj);
}

int dequeue_callback(st_pc_queue_t *queue, void *obj)
{
    return fprintf(dequeue_fp, "%d\n", (int)(long)obj);
}

typedef struct producer_args_t_ {
    st_pc_queue_t *queue;
    int id;
} producer_args_t;

static void* producer(void *args)
{
    producer_args_t *p_args;
    st_pc_queue_t *queue;
    int max_objs;
    int num_objs;
    int i;

    p_args = (producer_args_t *)args;
    queue = p_args->queue;

    max_objs = 2.5 * st_pc_queue_capacity(queue);
    num_objs = (int)st_random(0.5 * st_pc_queue_capacity(queue), max_objs);

    st_pc_queue_inc_producer(queue);
    for (i = 0; i < num_objs; i++) {
        if (st_pc_enqueue(queue, (void *)(long)(p_args->id * max_objs + i))
                != ST_PC_QUEUE_OK) {
            fprintf(stderr, "enqueue error");
            return NULL;
        }
    }
    st_pc_queue_dec_producer(queue);

    return NULL;
}

static void* consumer(void *args)
{
    st_pc_queue_t *queue;
    void *obj;
    int ret;

    queue = (st_pc_queue_t *)args;

    while (true) {
        ret = st_pc_dequeue(queue, &obj);
        if (ret == ST_PC_QUEUE_EMPTY) {
            break;
        } else if (ret != ST_PC_QUEUE_OK) {
            fprintf(stderr, "dequeue error");
            return NULL;
        }
    }

    return NULL;
}

static int test_seperate_one(st_pc_queue_t *queue, int num_producers,
        int num_consumers)
{
    pthread_t *pts = NULL;
    producer_args_t args;
    int i;

    st_pc_queue_clear(queue);
    if (!st_pc_queue_empty(queue)) {
        goto ERR;
    }

    pts = (pthread_t *)malloc((num_producers + num_consumers)
            * sizeof(pthread_t));
    if (pts == NULL) {
        goto ERR;
    }

    enqueue_fp = tmpfile();
    assert(enqueue_fp != NULL);
    args.queue = queue;
    for (i = 0; i < num_producers; i++) {
        args.id = i;
        if (pthread_create(pts + i, NULL, producer,
                    (void *)&args) != 0) {
            goto ERR;
        }
    }

    dequeue_fp = tmpfile();
    assert(dequeue_fp != NULL);
    for (i = 0; i < num_consumers; i++) {
        if (pthread_create(pts + num_producers + i, NULL, consumer,
                    NULL) != 0) {
            goto ERR;
        }
    }

    for (i = 0; i < num_producers + num_consumers; i++) {
        if (pthread_join(pts[i], NULL) != 0) {
            goto ERR;
        }
    }

    rewind(enqueue_fp);
    rewind(dequeue_fp);
    if (!diff_fp(enqueue_fp, dequeue_fp)) {
        goto ERR;
    }

    safe_fclose(enqueue_fp);
    safe_fclose(dequeue_fp);
    safe_free(pts);

    return 0;

ERR:

    safe_fclose(enqueue_fp);
    safe_fclose(dequeue_fp);
    safe_free(pts);

    return -1;
}

static int unit_test_seperate()
{
    st_pc_queue_t *queue = NULL;
    st_queue_id_t queue_size = 10;
    int ncase;

    fprintf(stderr, " Testing st_pc_queue seperate...\n");

    queue = st_pc_queue_create(queue_size, NULL);
    st_pc_set_enqueue_callback(queue, enqueue_callback);
    st_pc_set_dequeue_callback(queue, dequeue_callback);

    ncase = 1;
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    if (test_seperate_one(queue, 1, 1) != 0) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    fprintf(stderr, "Passed\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    if (test_seperate_one(queue, 1, 5) != 0) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    fprintf(stderr, "Passed\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    if (test_seperate_one(queue, 5, 1) != 0) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    fprintf(stderr, "Passed\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    if (test_seperate_one(queue, 5, 5) != 0) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    fprintf(stderr, "Passed\n");

    safe_st_pc_queue_destroy(queue);
    return 0;

FAILED:
    safe_st_pc_queue_destroy(queue);
    return -1;
}

static int run_all_tests()
{
    int ret = 0;

    if (unit_test_seperate() != 0) {
        ret = -1;
    }

    if (unit_test_combine() != 0) {
        ret = -1;
    }

    return ret;
}

int main(int argc, const char *argv[])
{
    int ret;

    st_srand(time(NULL));

    fprintf(stderr, "Start testing...\n");
    ret = run_all_tests();
    if (ret != 0) {
        fprintf(stderr, "Tests failed.\n");
    } else {
        fprintf(stderr, "Tests succeeded.\n");
    }

    return ret;
}
