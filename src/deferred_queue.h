/* openvpn-auth-http -- An OpenVPN plugin for do accounting.
 *
 * Copyright (C) ****** VPN Service 2014 <i.muzichuk@gmail.com>
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef _DEFERRED_QUEUE_
#define _DEFERRED_QUEUE_

#define DEFERRED_QUEUE_ADD_SUCCESS 0
#define DEFERRED_QUEUE_ADD_FULL -1

#define DEFERRED_QUEUE_MAX_SIZE 1024

struct deferred_queue_item;
typedef struct deferred_queue_item deferred_queue_item_t;

extern deferred_queue_item_t * create_deferred_queue_item(
    int (*handler)(void *handler_context),
    void (*context_cleaner)(void *handler_context),
    void *handler_context);
extern void free_deferred_queue_item(deferred_queue_item_t *item);

struct deferred_queue;
typedef struct deferred_queue deferred_queue_t;

extern deferred_queue_t * create_deferred_queue(void);
extern void free_deferred_queue(deferred_queue_t *queue);
extern int add_item_to_deferred_queue(deferred_queue_t *queue, deferred_queue_item_t *item);

#endif /* _DEFERRED_QUEUE_ */