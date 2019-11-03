#include "beeper.h"
#include <zephyr.h>
#include <stdint.h>
#include <drivers/pwm.h>
#include <device.h>
#include "alturia.h"
#include <logging/log.h>

LOG_MODULE_REGISTER(beeper, CONFIG_LOG_DEFAULT_LEVEL);
K_SEM_DEFINE(lock,1,1);

struct k_delayed_work work;

struct beep_sequenz_data {
    struct k_delayed_work work;
    uint32_t count;
    uint32_t pitch;
    int32_t delay;
} beep_sequenz;

static void work_handler(struct k_work *item)
{
    struct device *beeper = device_get_binding("PWM_2");
    struct device *led = device_get_binding(LED_GPIO_CONTROLLER);
    int res;

    if(beeper == NULL) {
        LOG_ERR("can not get beeper device");
        goto err_free_lock;
    }

    if(led == NULL) {
        LOG_ERR("can not get led device");
        goto err_free_lock;
    }

    struct k_delayed_work *dw =
        CONTAINER_OF(item, struct k_delayed_work, work);
    struct beep_sequenz_data *data =
        CONTAINER_OF(dw, struct beep_sequenz_data, work);

    if(data->count % 2) {
        res = pwm_pin_set_usec(beeper, 4, 500, 0);
        gpio_pin_write(led, LED_GPIO_PIN, false);
    } else {
        res = pwm_pin_set_usec(beeper, 4, 500, 10);
        gpio_pin_write(led, LED_GPIO_PIN, true);
    }

    if(res){
        goto err_free_lock;
    }

    data->count--;
    if(data->count > 0) {
        k_delayed_work_submit(dw, data->delay);
        return;
    }

err_free_lock:
    k_sem_give(&lock);
    return;
}

int beep(int32_t duration, int32_t pitch)
{
    return beepn(duration, 1, pitch);
}

int beepn(int32_t duration, int32_t count, int32_t pitch){
    if(k_sem_take(&lock, K_NO_WAIT)==0) {
        beep_sequenz.count = count*2;
        beep_sequenz.delay = duration;
        beep_sequenz.pitch = pitch;

        k_delayed_work_init(&beep_sequenz.work, work_handler);
        k_work_submit(&(beep_sequenz.work.work));
        return 0;
    }
    return -EBUSY;
}
