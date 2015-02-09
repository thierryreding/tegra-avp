#include <ctype.h>
#include <stdbool.h>
#include <types.h>

#include <avp/car.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pinmux.h>
#include <avp/uart.h>

void intc_init(void)
{
	unsigned long base = TEGRA_INTC_BASE;

	writel(0, base + 0x000);
	readl(base + 0x000);
}

void car_apply(void)
{
	unsigned long base = TEGRA_CLK_RST_BASE;
	unsigned long value;
	struct car car;

	car_init(&car, TEGRA_CLK_RST_BASE);

#if 0
	value = readl(base + 0x028);
	/* set clk_m as clock source */
	value &= ~((0x7 << 12) | (0x7 << 8) | (0x7 << 4) | (0x7 << 0));
	/* sys_state -> run */
	value &= ~(0xf << 28);
	value |= 2 << 28;
	writel(value, base + 0x028);

	/* get OSC frequency */
	value = readl(base + 0x050);
	value >>= 28;

	switch (value) {
	case 0: /* 13 MHz */
		/*
		 * pll_p
		 */
		value = readl(base + 0x0a0);
		/* divm */
		value &= ~(0x1f << 0);
		value |= 13 << 0;
		/* divn */
		value &= ~(0x3ff << 8);
		value |= 408 << 8;
		/* divp */
		value &= ~(0x7 << 20);
		value |= 0 << 20;
		/* allow base override */
		value |= 1 << 28;
		/* bypass */
		value |= 1 << 31;
		writel(value, base + 0x0a0);

		/* cpcon */
		value = readl(base + 0x0ac);
		value &= ~(0xf << 8);
		value |= 8 << 8;
		writel(value, base + 0x0ac);

		/* enable PLL */
		value = readl(base + 0x0a0);
		value |= 1 << 30;
		writel(value, base + 0x0a0);

		/* disallow base override */
		value = readl(base + 0x0a0);
		value &= ~(1 << 31);
		writel(value, base + 0x0a0);

		/*
		 * pll_c
		 */
		value = readl(base + 0x080);
		/* divm */
		value &= ~(0xff << 0);
		value |= 13 << 0;
		/* divn */
		value &= ~(0xff << 8);
		value |= 600 << 8;
		/* divp */
		value &= ~(0xf << 20);
		value |= 0 << 20;
		/* allow base override */
		value |= 1 << 28;
		/* bypass */
		value |= 1 << 31;
		writel(value, base + 0x080);

		/* cpcon */
		value = readl(base + 0x08c);
		value &= ~(0xf << 8);
		value |= 8 << 8;
		writel(value, base + 0x08c);

		/* enable PLL */
		value = readl(base + 0x080);
		value |= 1 << 30;
		writel(value, base + 0x080);

		/* disallow base override */
		value = readl(base + 0x080);
		value &= ~(1 << 31);
		writel(value, base + 0x080);
		break;

	case 8: /* 12 MHz */
		/*
		 * pll_p
		 */
		value = readl(base + 0x0a0);
		/* divm */
		value &= ~(0x1f << 0);
		value |= 12 << 0;
		/* divn */
		value &= ~(0x3ff << 8);
		value |= 408 << 8;
		/* divp */
		value &= ~(0x7 << 20);
		value |= 0 << 20;
		/* allow base override */
		value |= 1 << 28;
		/* bypass */
		value |= 1 << 31;
		writel(value, base + 0x0a0);

		/* cpcon */
		value = readl(base + 0x0ac);
		value &= ~(0xf << 8);
		value |= 8 << 8;
		writel(value, base + 0x0ac);

		/* enable PLL */
		value = readl(base + 0x0a0);
		value |= 1 << 30;
		writel(value, base + 0x0a0);

		/* disallow base override */
		value = readl(base + 0x0a0);
		value &= ~(1 << 31);
		writel(value, base + 0x0a0);

		/*
		 * pll_c
		 */
		value = readl(base + 0x080);
		/* divm */
		value &= ~(0xff << 0);
		value |= 12 << 0;
		/* divn */
		value &= ~(0xff << 8);
		value |= 456 << 8;
		/* divp */
		value &= ~(0xf << 20);
		value |= 1 << 20;
		/* allow base override */
		value |= 1 << 28;
		/* bypass */
		value |= 1 << 31;
		writel(value, base + 0x080);

		/* cpcon */
		value = readl(base + 0x08c);
		value &= ~(0xf << 8);
		value |= 8 << 8;
		writel(value, base + 0x08c);

		/* enable PLL */
		value = readl(base + 0x080);
		value |= 1 << 30;
		writel(value, base + 0x080);

		/* disallow base override */
		value = readl(base + 0x080);
		value &= ~(1 << 31);
		writel(value, base + 0x080);
		break;
	}

	/* PLL_P_OUT1, PLL_P_OUT2 */
	value = (0 << 16) | (0 << 0);
	writel(value, base + 0x0a4);

	value = (15 << 24) | (1 << 18) | (1 << 17) | (1 << 16) |
		(83 <<  8) | (1 <<  2) | (1 <<  1) | (1 <<  0);
	writel(value, base + 0x0a4);

	/* PLL_P_OUT3, PLL_P_OUT4 */
	value = (0 << 16) | (0 << 0);
	writel(value, base + 0x0a8);

	value = (2 << 24) | (1 << 18) | (1 << 17) | (1 << 16) |
		(6 <<  8) | (1 <<  2) | (1 <<  1) | (1 <<  0);
	writel(value, base + 0x0a4);

	value = readl(base + 0x028);
	/* set pll_p_out4 as clock source */
	value &= ~((0x7 << 12) | (0x7 << 8) | (0x7 << 4) | (0x7 << 0));
	value |= (2 << 12) | (2 << 8) | (2 << 4) | (2 << 0);
	/* sys_state -> run */
	value &= ~(0xf << 28);
	value |= 2 << 28;
	writel(value, base + 0x028);

	writel(0x00561600, base + 0x088);
	writel(0x01000000, base + 0x08c);
#endif

	/* disable UARTD clock */
	clk_periph_disable(&car, &clk_uartd);

	/* use pll_p_out0 as source */
	clk_periph_set_source(&car, &clk_uartd, 0);

	/* enable UARTD clock */
	clk_periph_enable(&car, &clk_uartd);

	/* assert reset */
	reset_assert(&car, &rst_uartd);

	/* deassert reset */
	reset_deassert(&car, &rst_uartd);
}

static const struct pinmux_config pinmux_configs[] = {
	/* PJ7 */
	PINMUX_CONFIG(0x3230, UARTD, NORMAL, NORMAL, DISABLE),
	/* PB0 */
	PINMUX_CONFIG(0x3234, UARTD, UP, NORMAL, ENABLE),
	/* PB1 */
	PINMUX_CONFIG(0x3238, UARTD, UP, NORMAL, ENABLE),
	/* PK7 */
	PINMUX_CONFIG(0x323c, UARTD, NORMAL, NORMAL, DISABLE),
};

void pinmux_apply(void)
{
	struct pinmux pinmux;
	unsigned int i;

	pinmux_init(&pinmux, TEGRA_APB_MISC_BASE);

	for (i = 0; i < ARRAY_SIZE(pinmux_configs); i++)
		pinmux_config_apply(&pinmux, &pinmux_configs[i]);
}

void do_irq(void)
{
	struct uart uart;

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "IRQ\n");
}

void do_fiq(void)
{
	struct uart uart;

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "FIQ\n");
}

void start(void)
{
	unsigned long value, i, j;
	struct uart uart;

	intc_init();
	pinmux_apply();
	car_apply();

	uart_init(&uart, TEGRA_UARTD_BASE, 115200);
	uart_puts(&uart, "AVP\n");

	uart_hexdump(&uart, (void *)TEGRA_IRAM_BASE, 60, 16, true);

	uart_printf(&uart, "AVP %s!\n", "foo");

	uart_printf(&uart, "foo: %02x\n", 0x05);

	while (1);
}
