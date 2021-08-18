#include "events2.h"
#include "logging/log.h"

LOG_MODULE_REGISTER(event2, 3);

DEFINE_EVENT(event_boot);
DEFINE_EVENT(event_timer_expire);

void event2_register_callback(event_t *event, event2_callback_t callback)
{
    struct event2_callback_node *node = &event->root;
    
    while (node->next != NULL) {
        node = node->next;
    }

    if (node != &event->root)
    {
        node->next = malloc(sizeof(struct event2_callback_node));
        node = node->next;
    }

    if(node == NULL) {
        LOG_ERR("unable to allocate memory for callback node");
        return;
    }

    node->next = NULL;
    node->callback = callback;
}

void event2_fire(event_t *event)
{
    struct event2_callback_node *node = &event->root;

    while(node->callback) {
        node->callback(event);
        node = node->next;

        if (node == NULL) {
            return;
        }
    }
}