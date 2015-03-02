#include <stdbool.h>

#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/timer.h>

#define TIMERUS_CNTR_1US 0x00
#define TIMERUS_USEC_CFG 0x04

void timer_us_init(enum osc_freq osc)
{
	unsigned int dividend, divisor;
	uint32_t value;

	switch (osc) {
	case OSC_FREQ_13:
	default:
		dividend = 1;
		divisor = 13;
		break;

	case OSC_FREQ_19_2:
		dividend = 5;
		divisor = 96;
		break;

	case OSC_FREQ_12:
		dividend = 1;
		divisor = 12;
		break;

	case OSC_FREQ_26:
		dividend = 1;
		divisor = 26;
		break;

	case OSC_FREQ_16_8:
		dividend = 5;
		divisor = 84;
		break;

	case OSC_FREQ_38_4:
		dividend = 5;
		divisor = 192;
		break;

	case OSC_FREQ_48:
		dividend = 1;
		divisor = 48;
		break;
	}

	value = ((dividend & 0xff) << 8) | (divisor & 0xff);
	writel(value, TEGRA_TMRUS_BASE + TIMERUS_USEC_CFG);
}

void udelay(unsigned long delay)
{
	unsigned long base = TEGRA_TMRUS_BASE;
	unsigned long value, start;

	start = readl(base + TIMERUS_CNTR_1US);

	while (true) {
		value = readl(base + TIMERUS_CNTR_1US);
		if (value > start + delay)
			break;
	}
}
