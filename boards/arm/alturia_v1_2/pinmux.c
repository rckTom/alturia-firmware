#include <kernel.h>
#include <device.h>
#include <init.h>
#include <drivers/pinmux.h>
#include <sys/sys_io.h>
#include <devicetree.h>

#include <pinmux/stm32/pinmux_stm32.h>
#include <pinmux/stm32/pinmux_stm32f4.h>

static const struct pin_config pinconf[] = {
#if DT_NODE_HAS_STATUS(DT_NODELABEL(usart1), okay)
	{STM32_PIN_PA9, STM32F4_PINMUX_FUNC_PA9_USART1_TX},
	{STM32_PIN_PA10, STM32F4_PINMUX_FUNC_PA10_USART1_RX},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(can2), okay)
	{STM32_PIN_PB5, STM32F4_PINMUX_FUNC_PB5_CAN2_RX},
	{STM32_PIN_PB6, STM32F4_PINMUX_FUNC_PB6_CAN2_TX},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi1), okay)
	{STM32_PIN_PA5, STM32F4_PINMUX_FUNC_PA5_SPI1_SCK},
	{STM32_PIN_PA6, STM32F4_PINMUX_FUNC_PA6_SPI1_MISO},
	{STM32_PIN_PA7, STM32F4_PINMUX_FUNC_PA7_SPI1_MOSI},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi2), okay)
	{STM32_PIN_PB13, STM32F4_PINMUX_FUNC_PB13_SPI2_SCK},
	{STM32_PIN_PB14, STM32F4_PINMUX_FUNC_PB14_SPI2_MISO},
	{STM32_PIN_PB15, STM32F4_PINMUX_FUNC_PB15_SPI2_MOSI},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(i2c2), okay)
	{STM32_PIN_PB10, STM32F4_PINMUX_FUNC_PB10_I2C2_SCL},
	{STM32_PIN_PB11, STM32F4_PINMUX_FUNC_PB11_I2C2_SDA},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(usbotg_fs), okay)
	{STM32_PIN_PA11, STM32F4_PINMUX_FUNC_PA11_OTG_FS_DM},
	{STM32_PIN_PA12, STM32F4_PINMUX_FUNC_PA12_OTG_FS_DP},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(pwm4), okay)
	{STM32_PIN_PB8, STM32F4_PINMUX_FUNC_PB8_PWM4_CH3},
	{STM32_PIN_PB9, STM32F4_PINMUX_FUNC_PB9_PWM4_CH4},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(pwm3), okay)
	{STM32_PIN_PC6, STM32F4_PINMUX_FUNC_PC6_PWM3_CH1},
	{STM32_PIN_PC7, STM32F4_PINMUX_FUNC_PC7_PWM3_CH2},
	{STM32_PIN_PC8, STM32F4_PINMUX_FUNC_PC8_PWM3_CH3},
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(pwm2), okay)
	{STM32_PIN_PA15, (STM32_PINMUX_ALT_FUNC_1 | STM32_PUSHPULL_PULLUP)}
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