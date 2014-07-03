#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "deferred_queue.h"
#include "plugin_utils.h"

static int handler(void *handler_context)
{
    int *value = (int*)handler_context;

    printf("HANDLER: handling %d and sleep\n", *value);
    usleep(40000);

    free(value);

    return 0;
}

static void handler_clean(void *handler_context)
{
}

int main(int argc, char **argv)
{
    deferred_queue_t *queue = create_deferred_queue();
    init_plugin_logging(4);

    for (int i = 0; i < 200; i++) {
        int *value = (int*)malloc(sizeof(int));
        *value = i + 1;
        deferred_queue_item_t *item = create_deferred_queue_item(&handler, &handler_clean, value);
        add_item_to_deferred_queue(queue, item);
    }


    for (int i = 0; i < 20000; i++) {
        int *value = (int*)malloc(sizeof(int));
        *value = i + 1;
        deferred_queue_item_t *item = create_deferred_queue_item(&handler, &handler_clean, value);
        if (add_item_to_deferred_queue(queue, item))
            printf("Queue is full\n");
    }

    printf("Sleeping...\n");
    sleep(10);
    printf("Wake up...\n");

    if (queue)
        free_deferred_queue(queue);
    clear_plugin_logging();
    return 0;
}