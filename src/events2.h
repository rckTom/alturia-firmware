#ifndef ALTURIA__EVENTS2__H
#define ALTURIA__EVENTS2__H

#include "stdlib.h"
#include <zephyr/zephyr.h>

#define DEFINE_EVENT(name) \
         STRUCT_SECTION_ITERABLE(event2, name) = { \
                 .evt_name = #name,\
                 .root = NULL, \
         }  \
            
#define DECLARE_EVENT(name) \
    extern struct event2 name;

typedef struct event2 event_t;
typedef void (*event2_callback_t)(event_t *evt);

struct event2_callback_node {
    event2_callback_t callback;
    struct event2_callback_node *next;
};

struct event2 {
    const char *evt_name;
    struct event2_callback_node *root;
};

void event2_register_callback(event_t *event, event2_callback_t callback);
void event2_fire(event_t *event);

#endif