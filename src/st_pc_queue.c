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
#include "st_pc_queue.h"

st_pc_queue_t* st_pc_queue_create(st_queue_id_t capacity, void *EOF_obj)
{
    st_pc_queue_t* queue = NULL;

    queue = (st_pc_queue_t*)malloc(sizeof(st_pc_queue_t));
    if(NULL == queue) {
        ST_WARNING("alloc memory for queue failed");
        return NULL;
    }
    memset(queue, 0, sizeof(st_pc_queue_t));

    queue->q = st_queue_create(capacity);
    if(queue->q == NULL) {
        ST_WARNING("Failed to st_queue_create.");
        goto FAILED;
    }

    if (st_pc_queue_clear(queue) < 0) {
        ST_WARNING("Failed to st_pc_queue_clear.");
        goto FAILED;
    }

    return queue;

FAILED:
    safe_st_pc_queue_destroy(queue);
    return NULL;
}

int st_pc_queue_inc_producer(st_pc_queue_t *queue)
{
    if (pthread_mutex_lock(&queue->producer_count_lock) != 0) {
        ST_WARNING("Failed to lock producer_count_lock");
        return ST_PC_QUEUE_ERR;
    }

    queue->producer_count++;

    if (pthread_mutex_unlock(&queue->producer_count_lock) != 0) {
        ST_WARNING("Failed to unlock producer_count_lock");
        return ST_PC_QUEUE_ERR;
    }

    return 0;
}

int st_pc_queue_dec_producer(st_pc_queue_t *queue)
{
    if (pthread_mutex_lock(&queue->producer_count_lock) != 0) {
        ST_WARNING("Failed to lock producer_count_lock");
        return ST_PC_QUEUE_ERR;
    }

    queue->producer_count--;

    if (pthread_mutex_unlock(&queue->producer_count_lock) != 0) {
        ST_WARNING("Failed to unlock producer_count_lock");
        return ST_PC_QUEUE_ERR;
    }

    if (st_pc_enqueue(queue, queue->EOF_obj) != ST_PC_QUEUE_OK) {
        ST_WARNING("Failed to st_pc_enqueue EOF_obj.");
        return ST_PC_QUEUE_ERR;
    }

    return 0;
}

int st_pc_enqueue(st_pc_queue_t* queue, void* obj)
{
    if (st_sem_wait(&queue->sem_empty) != 0) {
        ST_WARNING("Failed to st_sem_wait sem_empty.");
        return ST_PC_QUEUE_ERR;
    }
    if (pthread_mutex_lock(&queue->q_lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_lock q_lock.");
        return ST_PC_QUEUE_ERR;
    }

    if (st_enqueue(queue->q, obj) != ST_QUEUE_OK) {
        ST_WARNING("Failed to st_enqueue.");
        goto UNLOCK_AND_ERR;
    }

    if (queue->enqueue_callback != NULL) {
        if (queue->enqueue_callback(queue, obj) < 0) {
            ST_WARNING("Failed to call enqueue_callback.");
            goto UNLOCK_AND_ERR;
        }
    }

    if (pthread_mutex_unlock(&queue->q_lock) != 0) {
        ST_WARNING("Falied to pthread_mutex_unlock q_lock");
        return ST_PC_QUEUE_ERR;
    }
    if (st_sem_post(&queue->sem_fill) != 0) {
        ST_WARNING("Failed to st_sem_post sem_fill.");
        return ST_PC_QUEUE_ERR;
    }

    return ST_PC_QUEUE_OK;

UNLOCK_AND_ERR:
    (void) pthread_mutex_unlock(&queue->q_lock);
    return ST_PC_QUEUE_ERR;
}

static int st_pc_dequeue_one(st_pc_queue_t* queue, void** obj)
{
    if (st_sem_wait(&queue->sem_fill) != 0) {
        ST_WARNING("Failed to st_sem_wait sem_fill.");
        return ST_PC_QUEUE_ERR;
    }
    if (pthread_mutex_lock(&queue->q_lock) != 0) {
        ST_WARNING("Failed to pthread_mutex_lock q_lock.");
        return ST_PC_QUEUE_ERR;
    }

    if (st_dequeue(queue->q, obj) != ST_QUEUE_OK) {
        ST_WARNING("Failed to st_dequeue.");
        goto UNLOCK_AND_ERR;
    }

    if (queue->dequeue_callback != NULL) {
        if (queue->dequeue_callback(queue, *obj) < 0) {
            ST_WARNING("Failed to call dequeue_callback.");
            goto UNLOCK_AND_ERR;
        }
    }

    if (*obj == queue->EOF_obj) {
        if (st_pc_queue_empty(queue)) {
            if (pthread_mutex_lock(&queue->producer_count_lock) != 0) {
                ST_WARNING("Failed to lock producer_count_lock");
                goto UNLOCK_AND_ERR;
            }
            if (queue->producer_count == 0) {
                if (pthread_mutex_unlock(&queue->producer_count_lock) != 0) {
                    ST_WARNING("Failed to unlock producer_count_lock");
                    goto UNLOCK_AND_ERR;
                }
                if (pthread_mutex_unlock(&queue->q_lock) != 0) {
                    ST_WARNING("Failed to pthread_mutex_unlock q_lock.");
                    return ST_PC_QUEUE_ERR;
                }
                // put eof back to signal other consumers
                st_pc_enqueue(queue, queue->EOF_obj);
                return ST_PC_QUEUE_EMPTY;
            }
            if (pthread_mutex_unlock(&queue->producer_count_lock) != 0) {
                ST_WARNING("Failed to unlock producer_count_lock");
                goto UNLOCK_AND_ERR;
            }
        }
    }

    if (pthread_mutex_unlock(&queue->q_lock) != 0) {
        ST_WARNING("Falied to pthread_mutex_unlock q_lock");
        return ST_PC_QUEUE_ERR;
    }
    if (st_sem_post(&queue->sem_empty) != 0) {
        ST_WARNING("Failed to st_sem_post sem_empty.");
        return ST_PC_QUEUE_ERR;
    }

    return ST_PC_QUEUE_OK;

UNLOCK_AND_ERR:
    (void) pthread_mutex_unlock(&queue->q_lock);
    return ST_PC_QUEUE_ERR;
}

int st_pc_dequeue(st_pc_queue_t* queue, void** obj)
{
    int ret;

    while (true) {
        ret = st_pc_dequeue_one(queue, obj);
        if (ret != ST_PC_QUEUE_OK) { // ST_PC_QUEUE_EMPTY or ST_PC_QUEUE_ERR
            return ret;
        }

        if (*obj != queue->EOF_obj) { // absorb all EOF_obj
            break;
        }
    }

    return ST_PC_QUEUE_OK;
}

int st_pc_queue_empty(st_pc_queue_t* queue)
{
    return st_queue_empty(queue->q);
}

st_queue_id_t st_pc_queue_size(st_pc_queue_t* queue)
{
    return st_queue_size(queue->q);
}

st_queue_id_t st_pc_queue_capacity(st_pc_queue_t* queue)
{
    return st_queue_capacity(queue->q);
}

int st_pc_queue_clear(st_pc_queue_t* queue)
{
    queue->producer_count = 0;

    if (queue->inited) {
        if (pthread_mutex_destroy(&queue->producer_count_lock) != 0) {
            ST_WARNING("Cannot destroy producer_count_lock");
            return ST_PC_QUEUE_ERR;
        }
        if (pthread_mutex_destroy(&queue->q_lock) != 0) {
            ST_WARNING("Cannot destroy q_lock");
            return ST_PC_QUEUE_ERR;
        }
        if (st_sem_destroy(&queue->sem_empty) != 0) {
            ST_WARNING("Cannot destroy sem_empty");
            return ST_PC_QUEUE_ERR;
        }
        if (st_sem_destroy(&queue->sem_fill) != 0) {
            ST_WARNING("Cannot destroy sem_fill");
            return ST_PC_QUEUE_ERR;
        }
    }
    if (pthread_mutex_init(&queue->producer_count_lock, NULL) != 0) {
        ST_WARNING("Cannot initialize producer_count_lock");
        return ST_PC_QUEUE_ERR;
    }
    if (pthread_mutex_init(&queue->q_lock, NULL) != 0) {
        ST_WARNING("Cannot initialize q_lock");
        return ST_PC_QUEUE_ERR;
    }
    if (st_sem_init(&queue->sem_empty, st_queue_capacity(queue->q)) != 0) {
        ST_WARNING("Cannot initialize sem_empty");
        return ST_PC_QUEUE_ERR;
    }
    if (st_sem_init(&queue->sem_fill, 0) != 0) {
        ST_WARNING("Cannot initialize sem_fill");
        return ST_PC_QUEUE_ERR;
    }

    queue->inited = true;

    return st_queue_clear(queue->q);
}

void st_pc_queue_destroy(st_pc_queue_t* queue)
{
    if(queue == NULL) {
        return;
    }

    if (!queue->inited) {
        return;
    }

    queue->producer_count = 0;
    if (pthread_mutex_destroy(&queue->producer_count_lock) != 0) {
        ST_WARNING("Cannot initialize producer_count_lock");
        return;
    }
    safe_st_queue_destroy(queue->q);
}

void st_pc_set_enqueue_callback(st_pc_queue_t *queue,
        st_pc_queue_callback_t enqueue_callback)
{
    queue->enqueue_callback = enqueue_callback;
}

void st_pc_set_dequeue_callback(st_pc_queue_t *queue,
        st_pc_queue_callback_t dequeue_callback)
{
    queue->dequeue_callback = dequeue_callback;
}
