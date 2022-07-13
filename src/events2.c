#include "events2.h"
#include "logging/log.h"

LOG_MODULE_REGISTER(event2, 3);

DEFINE_EVENT(event_boot);
DEFINE_EVENT(event_timer_expire);

void event2_register_callback(event_t *event, event2_callback_t callback)
{
    struct event2_callback_node *new_node = malloc(sizeof(struct event2_callback_node));
    struct event2_callback_node *node = event->root;

    if (new_node == NULL) {
        LOG_ERR("unable to allocate memory for callback node");
        return;
    }

    new_node->callback = callback;
    new_node->next = NULL;

    if (event->root == NULL) {
        event->root = new_node;
        return;
    }

    while (node->next != NULL) {
        node = node->next;
    }

    node->next = new_node;
}

void event2_fire(event_t *event)
{
    struct event2_callback_node *node = event->root;

    while(1) {
        node->callback(event);
        node = node->next;

        if (node == NULL) {
            return;
        }
    }
}