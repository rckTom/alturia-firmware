#ifndef ALTURIA__EVENTS2__H
#define ALTURIA__EVENTS2__H

#include "stdlib.h"
#include "zephyr.h"

#define DEFINE_EVENT(name) \
         STRUCT_SECTION_ITERABLE(event2, name) = { \
                 .evt_name = #name,\
                 .root = {0}, \
         }  \
            


typedef void (*event2_callback_t)();

struct event2_callback_node {
    event2_callback_t callback;
    struct event2_callback_node *next;
};

struct event2 {
    const char *evt_name;
    struct event2_callback_node root;
};

typedef struct event2 event_t;

void event2_register_callback(event_t *event, event2_callback_t callback);
void event2_fire(event_t *event);

#endif