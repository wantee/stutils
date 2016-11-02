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

#ifndef  _ST_PC_QUEUE_H_
#define  _ST_PC_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include <stutils/st_macro.h>

#include "st_semaphore.h"
#include "st_queue.h"

/*
 * Usage:
 * 1) Seperate producers and consumers
 *
 * void* producer(void*)
 * {
 *     queue = ...;
 *     st_pc_queue_inc_producer(queue);
 *     while(...) {
 *         obj = produce();
 *         if (st_pc_enqueue(queue, obj) != ST_PC_QUEUE_OK) {
 *             err();
 *         }
 *     }
 *     st_pc_queue_dec_producer(queue);
 * }
 *
 * void* consumer(void*)
 * {
 *     queue = ...;
 *
 *     while (...) {
 *         ret = st_pc_dequeue(queue, &obj);
 *         if (ret == ST_PC_QUEUE_EMPTY) {
 *             // indicate that all producer have finished
 *             return NULL;
 *         } else if (ret != ST_PC_QUEUE_OK) {
 *             err();
 *         }
 *
 *         consume(obj);
 *     }
 * }
 *
 *
 * int main(int argc, char **argv)
 * {
 *     int num_producers = x;
 *     int num_consumers = y;
 *
 *     pthread_t *pt_producers = ...;
 *     pthread_t *pt_consumers = ...;
 *
 *     for (int i = 0; i < num_producers; i++) {
 *         pthread_create(pt_producers + i, NULL, producer, NULL);
 *     }
 *
 *     for (int i = 0; i < num_consumers; i++) {
 *         pthread_create(pt_consumers + i, NULL, consumer, NULL);
 *     }
 *
 *     for (int i = 0; i < num_producers; i++) {
 *         pthread_join(pt_producers[i]);
 *     }
 *
 *     st_pc_queue_stop(num_consumers);
 *
 *     for (int i = 0; i < num_consumers; i++) {
 *         pthread_join(pt_consumers[i]);
 *     }
 *
 *     return 0;
 * }
 *
 */

/*
 * 2) A thread acts as producer as well as consumer.
 *
 *
 * void* producer_consumer(void*)
 * {
 *     queue = ...;
 *
 *     ret = st_pc_dequeue(queue, &obj);
 *     if (ret == ST_PC_QUEUE_EMPTY) {
 *         // indicate that all producer have finished
 *         st_pc_queue_stop(num_consumers);
 *         return NULL;
 *     } else if (ret != ST_PC_QUEUE_OK) {
 *         err();
 *     }
 *
 *     consume(obj);
 *
 *     st_pc_queue_inc_producer(queue);
 *     int n = 0 or 1; // producer must produce less or equal than
 *                     // number of objs consumed before
 *     for (int i = 0; i < n; i++) {
 *         obj = produce();
 *         if (st_pc_enqueue(queue, obj) != ST_PC_QUEUE_OK) {
 *             err();
 *         }
 *     }
 *     st_pc_queue_dec_producer(queue);
 * }
 *
 *
 * int main(int argc, char **argv)
 * {
 *     int num_workers = x;
 *
 *     pthread_t *pts = ...;
 *
 *     obj = produce();
 *     st_pc_enqueue(queue, obj);
 *
 *     for (int i = 0; i < num_workers; i++) {
 *         pthread_create(pts + i, NULL, producer_consumer, NULL);
 *     }
 *
 *     for (int i = 0; i < num_workers; i++) {
 *         pthread_join(pts[i]);
 *     }
 *
 *     return 0;
 * }
 *
 */

#define ST_PC_QUEUE_OK    0
#define ST_PC_QUEUE_ERR   -1
#define ST_PC_QUEUE_FULL  1
#define ST_PC_QUEUE_EMPTY 2

typedef struct _st_producer_consumer_queue_t_ st_pc_queue_t;
typedef int (*st_pc_queue_callback_t)(st_pc_queue_t *queue, void *obj);

typedef struct _st_producer_consumer_queue_t_ {
    st_queue_t *q;

    st_sem_t sem_empty;
    st_sem_t sem_fill;
    pthread_mutex_t q_lock;

    bool inited;
    void *EOF_obj;
    int producer_count;
    pthread_mutex_t producer_count_lock;

    st_pc_queue_callback_t enqueue_callback;
    st_pc_queue_callback_t dequeue_callback;
} st_pc_queue_t;

st_pc_queue_t* st_pc_queue_create(st_queue_id_t capacity, void *EOF_obj);
#define safe_st_pc_queue_destroy(ptr) do {\
    if((ptr) != NULL) {\
        st_pc_queue_destroy(ptr);\
        safe_free(ptr);\
        (ptr) = NULL;\
    }\
    } while(0)
void st_pc_queue_destroy(st_pc_queue_t* queue);

void st_pc_set_enqueue_callback(st_pc_queue_t *queue,
        st_pc_queue_callback_t enqueue_callback);
void st_pc_set_dequeue_callback(st_pc_queue_t *queue,
        st_pc_queue_callback_t dequeue_callback);

/* following four functon must be called without race condition. */
st_queue_id_t st_pc_queue_capacity(st_pc_queue_t* queue);
st_queue_id_t st_pc_queue_size(st_pc_queue_t* queue);
int st_pc_queue_empty(st_pc_queue_t* queue);
int st_pc_queue_clear(st_pc_queue_t* queue);

int st_pc_queue_inc_producer(st_pc_queue_t *queue);
int st_pc_queue_dec_producer(st_pc_queue_t *queue);

int st_pc_enqueue(st_pc_queue_t* queue, void* obj);
int st_pc_dequeue(st_pc_queue_t* queue, void** obj);

#ifdef __cplusplus
}
#endif

#endif
