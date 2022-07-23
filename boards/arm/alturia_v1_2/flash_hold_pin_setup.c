#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

static int init_flash_hold_pin(const struct device *dev)
{
	(void)dev;
	const struct device* gpio = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(norflash), hold_gpios));
	const int pin = DT_GPIO_PIN(DT_ALIAS(norflash), hold_gpios);
	int rc = 0;

	if (gpio == NULL) {
		return rc;
	}

	rc = gpio_pin_configure(gpio, pin, GPIO_OUTPUT);
	if (rc != 0) {
		return rc;
	}

	rc = gpio_pin_set(gpio, pin, true);
	return 0;
}

SYS_INIT(init_flash_hold_pin, POST_KERNEL, 0);
