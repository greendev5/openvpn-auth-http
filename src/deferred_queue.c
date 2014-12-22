/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "deferred_queue.h"
#include "plugin_utils.h"

struct deferred_queue_item
{
    deferred_queue_item_t *next_node;

    int (*handler)(void *handler_context);
    void (*context_cleaner)(void *handler_context);
    void *handler_context;
};

struct deferred_queue
{
    pthread_mutex_t q_mutex;
    pthread_cond_t  q_condv;
    pthread_t       q_pthrd;

    unsigned char   q_sflag;

    size_t q_size;

    deferred_queue_item_t *q_head;
    deferred_queue_item_t *q_tail;
};

extern deferred_queue_item_t * create_deferred_queue_item(
    int (*handler)(void *handler_context),
    void (*context_cleaner)(void *handler_context),
    void *handler_context)
{
    deferred_queue_item_t *item = (deferred_queue_item_t*)malloc(sizeof(deferred_queue_item_t));
    item->next_node = NULL;
    item->handler = handler;
    item->context_cleaner = context_cleaner;
    item->handler_context = handler_context;
    return item;
}

extern void free_deferred_queue_item(deferred_queue_item_t *item)
{
    if (item->context_cleaner)
        item->context_cleaner(item->handler_context);
    free(item);
}

static deferred_queue_item_t * remove_item_from_deferred_queue(deferred_queue_t *queue)
{
    deferred_queue_item_t *item = NULL;

    pthread_mutex_lock(&queue->q_mutex);

    for (;;) {
        if (queue->q_sflag) {
            PLUGIN_DEBUG("Second thread got stop flag. Exiting...");
            item = NULL;
            goto out;
        } else if (queue->q_head) {
            item = queue->q_head;
            queue->q_head = item->next_node;
            if (!queue->q_head)
                queue->q_tail = NULL;
            queue->q_size--;
            goto out;
        }
        PLUGIN_DEBUG("Wait for deferred handler in queue...");
        pthread_cond_wait(&queue->q_condv, &queue->q_mutex);
    }

out:
    pthread_cond_signal(&queue->q_condv);
    pthread_mutex_unlock(&queue->q_mutex);
    return item;
}

static void halt_process()
{
}

static void * consumer_func(void *arg)
{
    int r = 0;
    deferred_queue_t *queue = (deferred_queue_t*)arg;
    deferred_queue_item_t *item = NULL;
    for (;;) {
        item = remove_item_from_deferred_queue(queue);
        if (item) {
            PLUGIN_DEBUG("Perform handler from queue...");
            r = item->handler(item->handler_context);
            free_deferred_queue_item(item);
            if (r < 0) {
                halt_process();
                break;
            }
        }
        else {
            break;
        }
    }
    return NULL;
}

extern deferred_queue_t * create_deferred_queue(void)
{
    int r;
    deferred_queue_t *queue = (deferred_queue_t*)malloc(sizeof(deferred_queue_t));
    memset(queue, 0, sizeof(deferred_queue_t));

    r = pthread_mutex_init(&queue->q_mutex, NULL);
    if (r) {
        goto error;
    }

    r = pthread_cond_init(&queue->q_condv, NULL);
    if (r) {
        pthread_mutex_destroy(&queue->q_mutex);
        goto error;
    }

    r = pthread_create(&queue->q_pthrd, NULL, consumer_func, queue);
    if (r) {
        pthread_mutex_destroy(&queue->q_mutex);
        pthread_cond_destroy(&queue->q_condv);
        goto error;
    }

    return queue;

error:
    free(queue);
    return NULL;
}

extern void free_deferred_queue(deferred_queue_t *queue)
{
    PLUGIN_DEBUG("Stoping second thread from manin thread...");
    pthread_mutex_lock(&queue->q_mutex);
    queue->q_sflag = 1;
    pthread_cond_signal(&queue->q_condv);
    pthread_mutex_unlock(&queue->q_mutex);

    pthread_join(queue->q_pthrd, NULL);

    PLUGIN_DEBUG("Destroing mutex and condition...");
    pthread_cond_destroy(&queue->q_condv);
    pthread_mutex_destroy(&queue->q_mutex);

    PLUGIN_DEBUG("Free queue items mem...");
    deferred_queue_item_t *item;
    while (queue->q_head) {
        item = queue->q_head;
        queue->q_head = queue->q_head->next_node;
        free_deferred_queue_item(item);
    }

    PLUGIN_DEBUG("Free queue mem...");
    free(queue);
}

extern int add_item_to_deferred_queue(deferred_queue_t *queue, deferred_queue_item_t *item)
{
    int r = DEFERRED_QUEUE_ADD_SUCCESS;
   
    PLUGIN_DEBUG("Start - add_item_to_deferred_queue"); 
    pthread_mutex_lock(&queue->q_mutex);
    
    if (queue->q_size > DEFERRED_QUEUE_MAX_SIZE) {
        PLUGIN_ERROR("Deferred QUEUE is FULL!!!");
        r = DEFERRED_QUEUE_ADD_FULL;
        goto out;
    }
    
    if (!queue->q_head) {
        queue->q_head = item;
    } else {
        queue->q_tail->next_node = item;
    }
    queue->q_tail = item;
    queue->q_size++;
    pthread_cond_signal(&queue->q_condv);

out:
    pthread_mutex_unlock(&queue->q_mutex);
    PLUGIN_DEBUG("END - add_item_to_deferred_queue");
    return r;
}
