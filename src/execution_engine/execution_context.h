#ifndef ALTURIA_EXECUTION_CONTEXT_H
#define ALTURIA_EXECUTION_CONTEXT_H

#include <stddef.h>
#include "generic.h"
#include "actions.h"
#include "events.h"
#include "variable.h"

#define conf_timer(conf_des) (void *)((char*)(conf_des) + sizeof(struct conf_desc))
#define conf_burnout(conf_desc) (void *)((char*)conf_timer(conf_desc) + (conf_desc->num_timer) * sizeof(struct timer_evt))
#define conf_apogee(conf_desc) (void *)((char*)conf_burnout(conf_desc) + (conf_desc->num_burnout) * sizeof(struct burnout_evt))
#define conf_ignition(conf_desc) (void *)((char*)conf_apogee(conf_desc) + conf_desc->num_apogee * sizeof(struct apogee_evt))
#define conf_touchdown(conf_desc) (void *)((char*)conf_ignition(conf_desc) + conf_desc->num_ignition * sizeof(struct ignition_evt))
#define conf_boot(conf_desc) (void *)((char*)conf_touchdown(conf_desc) + conf_desc->num_touchdown * sizeof(struct touchdown_evt))
#define conf_tick(conf_desc) (void *)((char*)conf_boot(conf_desc) + conf_desc->num_boot * sizeof(struct boot_evt))

#define conf_vars(conf_desc) (void *)((char*)conf_tick(conf_desc) + conf_desc->num_tick * sizeof(struct tick_evt))
#define conf_actions(conf_desc) (void*)((char*)conf_vars(conf_desc) + conf_desc->num_vars * sizeof(struct variable))
#define conf_pyro_set_actions(conf_desc) (void *)((char*)conf_actions(conf_desc) + conf_desc->num_actions * sizeof(struct action))
#define conf_pyro_read_actions(conf_desc) (void *)((char*)conf_pyro_set_actions(conf_desc) + conf_desc->num_pyro_set_actions * sizeof(struct pyro_set_action))
#define conf_servo_set_actions(conf_desc) (void *)((char*)conf_pyro_read_actions(conf_desc) + conf_desc->num_pyro_read_actions * sizeof(struct pyro_read_action))
#define conf_servo_read_actions(conf_desc) (void *)((char*)conf_servo_set_actions(conf_desc) + conf_desc->num_servo_set_actions * sizeof(struct servo_set_action))
#define conf_beep_action(conf_desc) (void *)((char*)conf_servo_read_actions(conf_desc) + conf_desc->num_servo_read_actions * sizeof(struct servo_read_action))
#define conf_timer_start_action(conf_desc) (void *)((char*)conf_beep_action(conf_desc) + conf_desc->num_beep_actions * sizeof(struct beep_action))
#define conf_timer_stop_action(conf_desc) (void *)((char*)conf_timer_start_action(conf_desc) + conf_desc->num_timer_start_actions * sizeof(struct timer_start_action))
#define conf_led_set_action(conf_desc) (void *)((char *)conf_timer_stop_action(conf_desc) + conf_desc->num_timer_stop_actions * sizeof(struct timer_stop_action))


struct conf_desc {
	uint32_t revision;
	
	uint32_t num_actions;
	uint32_t num_vars;
	uint32_t num_pyro_set_actions;
	uint32_t num_pyro_read_actions;
	uint32_t num_servo_set_actions;
	uint32_t num_servo_read_actions;
	uint32_t num_beep_actions;
	uint32_t num_timer_start_actions;
	uint32_t num_timer_stop_actions;
	uint32_t num_led_set_actions;

    uint8_t num_timer;
	uint8_t num_burnout;
	uint8_t num_apogee;
	uint8_t num_ignition;
	uint8_t num_touchdown;
	uint8_t num_boot;
	uint8_t num_tick;
} __attribute__((packed));

struct action* get_action(const struct conf_desc *ctx, unsigned int idx); 
struct variable* get_var(const struct conf_desc *ctx, int var_idx);
struct timer_evt* get_timer(const struct conf_desc *ctx, unsigned int timer_number);
struct burnout_evt* get_burnout(const struct conf_desc *ctx, unsigned int burnout_number);
struct ignition_evt* get_ignition_evt(const struct conf_desc *ctx, unsigned int ignition_number);
struct timer_start_action *get_timer_start_action(const struct conf_desc *ctx, unsigned int idx);
struct timer_stop_action *get_timer_stop_action(const struct conf_desc *ctx, unsigned int idx);
struct pyro_set_action *get_pyro_set_action(const struct conf_desc *ctx, unsigned int idx);
struct pyro_read_action *get_pyro_read_action(const struct conf_desc *ctx, unsigned int idx);
struct servo_read_action *get_servo_read_action(const struct conf_desc *ctx, unsigned int idx);
struct servo_set_action *get_servo_set_action(const struct conf_desc *ctx, unsigned int idx);
struct beep_action *get_beep_action(const struct conf_desc *ctx, unsigned int idx);
struct led_set_action *get_led_set_action(const struct conf_desc *ctx, unsigned int idx);
#endif

