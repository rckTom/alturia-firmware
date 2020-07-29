#ifndef ALTURIA_SERVOS_H
#define ALTURIA_SERVOS_H

#include <zephyr.h>

int servo_set_max_us(uint8_t servo, uint32_t max_us);
int servo_set_min_us(uint8_t servo, uint32_t min_us);
int servo_set_max_rate(int servo, uint32_t max_rate);
int servo_set_angle(int servo, uint8_t angle);
int servo_set_us(int servo, uint32_t us);


#endif
