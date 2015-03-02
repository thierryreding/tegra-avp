#include <ctype.h>
#include <irq.h>
#include <nv3p.h>
#include <stdbool.h>
#include <types.h>

#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pinmux.h>
#include <avp/pmc.h>
#include <avp/timer.h>
#include <avp/uart.h>
#include <avp/usb.h>

#define GEN1_I2C_SDA 0x31a0
#define GEN1_I2C_SCL 0x31a4

#define GPIO_PI3 0x31e8
#define GPIO_PG4 0x3200
#define GPIO_PG5 0x3204
#define GPIO_PG6 0x3208
#define GPIO_PG7 0x320c

#define GPIO_PJ7 0x3230
#define GPIO_PB0 0x3234
#define GPIO_PB1 0x3238
#define GPIO_PK7 0x323c

#define GEN2_I2C_SCL 0x3250
#define GEN2_I2C_SDA 0x3254

#define SDMMC4_CLK 0x3258
#define SDMMC4_CMD 0x325c
#define SDMMC4_DAT0 0x3260
#define SDMMC4_DAT1 0x3264
#define SDMMC4_DAT2 0x3268
#define SDMMC4_DAT3 0x326c
#define SDMMC4_DAT4 0x3270
#define SDMMC4_DAT5 0x3274
#define SDMMC4_DAT6 0x3278
#define SDMMC4_DAT7 0x327c

#define PWR_I2C_SCL 0x32b4
#define PWR_I2C_SDA 0x32b8

#define KB_ROW7 0x32d8
#define KB_ROW8 0x32dc
#define KB_ROW9 0x32e0
#define KB_ROW10 0x32e4

static const struct pinmux_config pinmux_configs[] = {
	PINMUX_CONFIG(GEN1_I2C_SDA, I2C1,   NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GEN1_I2C_SCL, I2C1,   NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(GEN2_I2C_SCL, I2C2,   NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GEN2_I2C_SDA, I2C2,   NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(PWR_I2C_SCL,  I2CPWR, NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(PWR_I2C_SDA,  I2CPWR, NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(SDMMC4_CLK,   SDMMC4, NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_CMD,   SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT0,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT1,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT2,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT3,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT4,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT5,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT6,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(SDMMC4_DAT7,  SDMMC4, UP,     NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(GPIO_PJ7,     UARTD,  NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PB0,     UARTD,  UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PB1,     UARTD,  UP,     NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PK7,     UARTD,  NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),

	PINMUX_CONFIG(GPIO_PG4,     SPI4,   NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PG5,     SPI4,   NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PG6,     SPI4,   NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PG7,     SPI4,   NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GPIO_PI3,     SPI4,   NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),

	PINMUX_CONFIG(KB_ROW7,      UARTA,  NORMAL, NORMAL, DISABLE, DISABLE, DISABLE),
	PINMUX_CONFIG(KB_ROW8,      UARTA,  NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(KB_ROW9,      UARTA,  NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(KB_ROW10,     UARTA,  NORMAL, NORMAL, ENABLE,  DISABLE, DISABLE),
};

void pinmux_apply(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pinmux_configs); i++)
		pinmux_config_apply(&pinmux, &pinmux_configs[i]);
}

void car_apply(void)
{
	/* use pll_p_out0 as source */
	clk_periph_set_source(&clk_uartd, 0);
	clk_periph_enable(&clk_uartd);
	reset_assert(&rst_uartd);
	reset_deassert(&rst_uartd);

	/*
	clk_periph_enable(&car, &clk_usbd);
	reset_assert(&car, &rst_usbd);
	reset_deassert(&car, &rst_usbd);
	*/
}

#define RELEASE "2015.02-wip"

void start(void)
{
	struct nv3p nv3p;

	/* this seems to fix memory corruption seen during USB transfers */
	if (1) {
		uint32_t value;

		value = readl(0x6000c000 + 0x000);
		value |= 1 << 4;
		writel(value, 0x6000c000 + 0x000);
	}

	//irq_init();
	pinmux_apply();
	car_apply();

	uart_init(debug);
	uart_puts(debug, "AVP " RELEASE "\n");

	usb_init(&usbd);
	usb_enumerate(&usbd);

	nv3p_init(&nv3p, &usbd);
	nv3p_process(&nv3p);

	uart_flush(debug);
	pmc_reset(&pmc, PMC_RESET_MODE_RCM);

	while (1);
}
