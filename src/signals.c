#include "signals.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "sdbm.h"
#include "logging/log.h"

LOG_MODULE_REGISTER(signal, 4);

#define TABLE_SIZE 512
static struct signal signal_table[TABLE_SIZE] = {0};

static uint32_t get_id(const char *key)
{
    return sdbm(key) % TABLE_SIZE;
}

int signal_insert(const char *name, struct generic_ptr value)
{
    uint32_t idx = get_id(name);
    LOG_INF("insert %s with key %d", name, idx);
    
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (signal_table[idx].value.value_ptr.uint32 == NULL) {
            uint32_t len = strlen(name) + 1;
            signal_table[idx].name = malloc(len);

            if (signal_table[idx].name == NULL) {
                return -1;
            }
            
            k_sched_lock();
            memcpy(signal_table[idx].name, name, len);
            signal_table[idx].value = value;
            k_sched_unlock();
            return 0;
        }

        idx++;
        idx = idx % TABLE_SIZE;
    }

    return -1;
}

struct generic_ptr signal_get(const char *name)
{
    uint32_t idx = get_id(name);
    
    for (int i = 0; i< TABLE_SIZE; i++) {
        if (signal_table[idx].value.value_ptr.uint32 == NULL || strcmp(signal_table[idx].name, name) == 0) {
            return signal_table[idx].value;
        }

        idx++;
        idx = idx % TABLE_SIZE;
    }
}