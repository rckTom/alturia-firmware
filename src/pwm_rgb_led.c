#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

#define DT_DRV_COMPAT rgb_pwm_led

LOG_MODULE_REGISTER(rgb_pwm_led, CONFIG_LOG_DEFAULT_LEVEL);

struct rgb_pwm_led_config {
	const struct pwm_dt_spec r, g, b;
};

struct rgb_pwm_led_data {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t brightness;
};

static int rgb_pwm_led_off(const struct device *dev, uint32_t led)
{
	const struct rgb_pwm_led_config *cfg = dev->config;
	int ret;

	ret = pwm_set_pulse_dt(&cfg->r, 0);
	if (ret != 0) {
		return ret;
	}

	ret = pwm_set_pulse_dt(&cfg->g, 0);
	if (ret != 0) {
		return ret;
	}

	return pwm_set_pulse_dt(&cfg->b, 0);
}

static int rgb_pwm_led_set_color(const struct device *dev, uint32_t led,
				 uint8_t num_colors, const uint8_t *color)
{
	const struct rgb_pwm_led_config *cfg = dev->config;
	struct rgb_pwm_led_data *data = dev->data;
	uint32_t pulse_r, pulse_g, pulse_b;
	int ret;

	if (num_colors != 3) {
		return -EINVAL;
	}

	pulse_r = cfg->r.period * color[0] / 0xFF;
	pulse_g = cfg->g.period * color[1] / 0xFF;
	pulse_b = cfg->b.period * color[2] / 0xFF;

	ret = pwm_set_pulse_dt(&cfg->r, pulse_r);
	if (ret != 0) {
		return ret;
	}
	data->r = color[0];

	ret = pwm_set_pulse_dt(&cfg->g, pulse_g);
	if (ret != 0) {
		return ret;
	}
	data->g = color[1];

	ret = pwm_set_pulse_dt(&cfg->b, pulse_b);
	if (ret != 0) {
		return ret;
	}
	data->b = color[2];

	return 0;
}

static int rgb_pwm_led_set_brightness(const struct device *dev, uint32_t led,
				      uint8_t value)
{
	struct rgb_pwm_led_data *data = dev->data;

	data->brightness = value;
	return rgb_pwm_led_set_color(dev, led, 3, &data->r);
}

static int rgb_pwm_led_on(const struct device *dev, uint32_t led)
{
	struct rgb_pwm_led_data *data = dev->data;
	return rgb_pwm_led_set_color(dev, led, 3, &data->r);
}

int rgb_pwm_led_init(const struct device *dev)
{
	// check if pwm devices are ready
	const struct rgb_pwm_led_config *cfg = dev->config;
	if (!device_is_ready(cfg->r.dev) || !device_is_ready(cfg->g.dev) ||
	    !device_is_ready(cfg->b.dev)) {
		LOG_ERR("pwm devices for rgb led are not ready");
		return -ENODEV;
	}

	return rgb_pwm_led_off(dev, 0);
}

const struct led_driver_api api = {
    .on = rgb_pwm_led_on,
    .off = rgb_pwm_led_off,
    .set_brightness = rgb_pwm_led_set_brightness,
    .set_color = rgb_pwm_led_set_color,
};

#define RGB_PWM_LED_DEVICE(i)                                                  \
                                                                               \
	static const struct rgb_pwm_led_config rgb_pwm_led_config_##i = {      \
	    .r = PWM_DT_SPEC_GET_BY_NAME(DT_INST(i, DT_DRV_COMPAT), r),        \
	    .g = PWM_DT_SPEC_GET_BY_NAME(DT_INST(i, DT_DRV_COMPAT), g),        \
	    .b = PWM_DT_SPEC_GET_BY_NAME(DT_INST(i, DT_DRV_COMPAT), b),        \
	};                                                                     \
                                                                               \
	static struct rgb_pwm_led_data rbg_pwm_led_data_##i = {                \
	    .r = 0,                                                            \
	    .g = 0,                                                            \
	    .b = 0,                                                            \
	    .brightness = 0xFF,                                                \
	};                                                                     \
                                                                               \
	DEVICE_DT_INST_DEFINE(i, &rgb_pwm_led_init, NULL,                      \
			      &rbg_pwm_led_data_##i, &rgb_pwm_led_config_##i,  \
			      APPLICATION, 0, &api);

DT_INST_FOREACH_STATUS_OKAY(RGB_PWM_LED_DEVICE);