/*
 * Copyright (c) 2017 I-SENSE group of ICCS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <device.h>
#include <init.h>
#include <drivers/pinmux.h>
#include <sys/sys_io.h>
#include <devicetree.h>
#include <pinmux/stm32/pinmux_stm32.h>

/* pin assignments for STM32F3DISCOVERY board */
static const struct pin_config pinconf[] = {
	{STM32_PIN_PA9, STM32F3_PINMUX_FUNC_PA9_USART1_TX},
	{STM32_PIN_PA10, STM32F3_PINMUX_FUNC_PA10_USART1_RX},
#ifdef CONFIG_PWM
	{STM32_PIN_PB11, (STM32_PINMUX_ALT_FUNC_1 | STM32_PUSHPULL_NOPULL)},
	{STM32_PIN_PB8,  (STM32_PINMUX_ALT_FUNC_10 | STM32_PUSHPULL_NOPULL)},
	{STM32_PIN_PB9,  (STM32_PINMUX_ALT_FUNC_10 | STM32_PUSHPULL_NOPULL)},

#endif
	{STM32_PIN_PB13, STM32F3_PINMUX_FUNC_PB13_SPI2_SCK},
	{STM32_PIN_PB14, STM32F3_PINMUX_FUNC_PB14_SPI2_MISO},
	{STM32_PIN_PB15, STM32F3_PINMUX_FUNC_PB15_SPI2_MOSI},
	{STM32_PIN_PA5,  STM32F3_PINMUX_FUNC_PA5_SPI1_SCK},
	{STM32_PIN_PA6,  STM32F3_PINMUX_FUNC_PA6_SPI1_MISO},
	{STM32_PIN_PA7,  STM32F3_PINMUX_FUNC_PA7_SPI1_MOSI},

#ifdef CONFIG_USB
	{STM32_PIN_PA11, STM32F3_PINMUX_FUNC_PA11_USB_DM},
	{STM32_PIN_PA12, STM32F3_PINMUX_FUNC_PA12_USB_DP},
#endif
};

static int pinmux_stm32_init(struct device *port)
{
	ARG_UNUSED(port);

	stm32_setup_pins(pinconf, ARRAY_SIZE(pinconf));

	return 0;
}
SYS_INIT(pinmux_stm32_init, PRE_KERNEL_1,
		CONFIG_PINMUX_STM32_DEVICE_INITIALIZATION_PRIORITY);
