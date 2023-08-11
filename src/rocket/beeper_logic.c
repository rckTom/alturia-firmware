#include "beeper_logic.h"
#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include "events2.h"
#include "flightstate.h"
#include "beeper.h"

static K_THREAD_STACK_DEFINE(stack, 512);
static struct k_thread beeper_logic_thread;

static void rocket_beeper_logic_worker(void *p1, void *p2, void *p3)
{
    while (1) {
        mission_state_t state = flightstate_get_state();

        switch (state) {
            case STATE_PAD_IDLE:
                beep(50, BEEP_FREQUENCY_HZ(1000));
                k_sleep(K_SECONDS(2));
            break;
            case STATE_PAD_READY:
                beep(50, BEEP_FREQUENCY_HZ(3000));
                k_sleep(K_SECONDS(5));
            break;

            case STATE_LANDED:
                beep(100, BEEP_FREQUENCY_HZ(2000));
                k_sleep(K_MSEC(800));
            break;

            case STATE_ERROR:

            break;
            default:
        };
    }
}

void rocket_beeper_logic_start() {
    k_tid_t tid = k_thread_create(&beeper_logic_thread, stack, K_THREAD_STACK_SIZEOF(stack),
                    (k_thread_entry_t) rocket_beeper_logic_worker,
                    NULL,
                    NULL,
                    NULL,
                    10,
                    0,
                    K_NO_WAIT);
    k_thread_name_set(tid, "beeper_logic");
}

