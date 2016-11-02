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
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "st_log.h"
#include "st_rand.h"

#include "st_pc_queue.h"

static bool diff_fp(FILE *enqueue_fp, FILE *dequeue_fp)
{
    int a;
    int b;

    while (true) {
        a = fgetc(enqueue_fp);
        b = fgetc(dequeue_fp);

        if (b == EOF) {
            break;
        }

        if (a != b) {
            return false;
        }
    }

    // enqueue_fp must have a extra EOF_obj
    if (a != '0') {
        return false;
    }
    if (fgetc(enqueue_fp) != '\n') {
        return false;
    }
    if (fgetc(enqueue_fp) != EOF) {
        return false;
    }

    return true;
}

static FILE *enqueue_fp = NULL;
static FILE *dequeue_fp = NULL;

#ifdef _PC_QUEUE_TEST_DEBUG_
static int ncase = 0;
#endif

static int enqueue_callback(st_pc_queue_t *queue, void *obj)
{
    return fprintf(enqueue_fp, "%d\n", (int)(long)obj);
}

static int dequeue_callback(st_pc_queue_t *queue, void *obj)
{
    return fprintf(dequeue_fp, "%d\n", (int)(long)obj);
}

static int produce(st_pc_queue_t *queue, int start_id, int num_objs)
{
    int i;

    st_pc_queue_inc_producer(queue);
    for (i = 0; i < num_objs; i++) {
        usleep((unsigned int)st_random(0, 10));
        if (st_pc_enqueue(queue, (void *)(long)(start_id + i + 1))
                != ST_PC_QUEUE_OK) {
            fprintf(stderr, "enqueue error");
            return -1;
        }
    }
    st_pc_queue_dec_producer(queue);

    return 0;
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

    p_args = (producer_args_t *)args;
    queue = p_args->queue;

    max_objs = 2 * st_pc_queue_capacity(queue);
    num_objs = (int)st_random(0.25 * max_objs, max_objs);

    produce(queue, p_args->id * max_objs, num_objs);

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
    producer_args_t *args = NULL;
    int i;

    st_pc_queue_clear(queue);
    if (!st_pc_queue_empty(queue)) {
        goto ERR;
    }

    pts = (pthread_t *)malloc((num_producers + num_consumers)
            * sizeof(pthread_t));
    assert(pts != NULL);

    args = (producer_args_t *)calloc(num_producers, sizeof(producer_args_t));
    assert(args != NULL);

#ifdef _PC_QUEUE_TEST_DEBUG_
    {
        char file[128];
        snprintf(file, 128, "en-%d.txt", ncase);
        enqueue_fp = fopen(file, "w+");
        snprintf(file, 128, "de-%d.txt", ncase);
        dequeue_fp = fopen(file, "w+");
        ncase++;
    }
#else
    enqueue_fp = tmpfile();
    dequeue_fp = tmpfile();
#endif
    assert(enqueue_fp != NULL);
    setlinebuf(enqueue_fp);
    assert(dequeue_fp != NULL);
    setlinebuf(dequeue_fp);
    for (i = 0; i < num_producers; i++) {
        args[i].queue = queue;
        args[i].id = i;
        if (pthread_create(pts + i, NULL, producer,
                    (void *)(args + i)) != 0) {
            goto ERR;
        }
    }

    for (i = 0; i < num_consumers; i++) {
        if (pthread_create(pts + num_producers + i, NULL, consumer,
                    (void *)queue) != 0) {
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
    safe_free(args);

    return 0;

ERR:

    safe_fclose(enqueue_fp);
    safe_fclose(dequeue_fp);
    safe_free(pts);
    safe_free(args);

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

typedef struct worker_args_t_ {
    st_pc_queue_t *queue;
    int id;
    int num_prod_steps;
    int max_prod_step;
} worker_args_t;

static void* producer_consumer(void *args)
{
    worker_args_t *w_args;
    st_pc_queue_t *queue;
    void *obj;

    int ret;
    int max_objs;
    int num_objs;

    w_args = (worker_args_t *)args;
    queue = w_args->queue;

    while (true) {
        ret = st_pc_dequeue(queue, &obj);
        if (ret == ST_PC_QUEUE_EMPTY) {
            break;
        } else if (ret != ST_PC_QUEUE_OK) {
            fprintf(stderr, "dequeue error");
            return NULL;
        }

        if (w_args->max_prod_step <= 0) {
            w_args->max_prod_step = (int)st_random(1, 10);
        }
        if (w_args->num_prod_steps >= w_args->max_prod_step) {
            continue;
        } else {
            max_objs = 1;
            num_objs = 1;

            produce(queue, w_args->id * max_objs, num_objs);
            w_args->num_prod_steps++;
        }
    }

    return NULL;
}

static int test_combine_one(st_pc_queue_t *queue, int num_workers)
{
    pthread_t *pts = NULL;
    worker_args_t *args = NULL;
    int i;

    st_pc_queue_clear(queue);
    if (!st_pc_queue_empty(queue)) {
        goto ERR;
    }
    pts = (pthread_t *)malloc((num_workers) * sizeof(pthread_t));
    assert(pts != NULL);

#ifdef _PC_QUEUE_TEST_DEBUG_
    {
        char file[128];
        snprintf(file, 128, "en-%d.txt", ncase);
        enqueue_fp = fopen(file, "w+");
        snprintf(file, 128, "de-%d.txt", ncase);
        dequeue_fp = fopen(file, "w+");
        ncase++;
    }
#else
    enqueue_fp = tmpfile();
    dequeue_fp = tmpfile();
#endif
    assert(enqueue_fp != NULL);
    assert(dequeue_fp != NULL);
    setlinebuf(enqueue_fp);
    setlinebuf(dequeue_fp);

    if (st_pc_enqueue(queue, (void *)-1) != ST_PC_QUEUE_OK) {
        goto ERR;
    }

    args = (worker_args_t *)calloc(num_workers, sizeof(worker_args_t));
    assert(args != NULL);
    for (i = 0; i < num_workers; i++) {
        args[i].queue = queue;
        args[i].id = i;
        if (pthread_create(pts + i, NULL, producer_consumer,
                    (void *)(args + i)) != 0) {
            goto ERR;
        }
    }

    for (i = 0; i < num_workers; i++) {
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
    safe_free(args);

    return 0;

ERR:

    safe_fclose(enqueue_fp);
    safe_fclose(dequeue_fp);
    safe_free(pts);
    safe_free(args);

    return -1;
}

static int unit_test_combine()
{
    st_pc_queue_t *queue = NULL;
    st_queue_id_t queue_size = 10;
    int ncase;

    fprintf(stderr, " Testing st_pc_queue combine...\n");

    queue = st_pc_queue_create(queue_size, NULL);
    st_pc_set_enqueue_callback(queue, enqueue_callback);
    st_pc_set_dequeue_callback(queue, dequeue_callback);

    ncase = 1;
    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    if (test_combine_one(queue, 1) != 0) {
        fprintf(stderr, "Failed\n");
        goto FAILED;
    }
    fprintf(stderr, "Passed\n");

    /*****************************************/
    fprintf(stderr, "    Case %d...", ncase++);
    if (test_combine_one(queue, 5) != 0) {
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
    st_log_opt_t log_opt = {
        .file = "/dev/stderr",
        .level = 8,
    };
    int ret;

    st_srand(time(NULL));

    st_log_open_mt(&log_opt);

    fprintf(stderr, "Start testing...\n");
    ret = run_all_tests();
    if (ret != 0) {
        fprintf(stderr, "Tests failed.\n");
    } else {
        fprintf(stderr, "Tests succeeded.\n");
    }

    return ret;
}
