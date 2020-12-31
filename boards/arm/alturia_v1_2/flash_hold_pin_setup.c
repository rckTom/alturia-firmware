#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

static void init_flash_hold_pin()
{
	const struct device* gpio = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(norflash), hold_gpios));
	const int pin = DT_GPIO_PIN(DT_ALIAS(norflash), hold_gpios);
	int rc = 0;

	if (gpio == NULL) {
		return;
	}

	rc = gpio_pin_configure(gpio, pin, GPIO_OUTPUT);
	if (rc != 0) {
		return;
	}

	rc = gpio_pin_set(gpio, pin, true);	
}

SYS_INIT(init_flash_hold_pin, POST_KERNEL, 0);
