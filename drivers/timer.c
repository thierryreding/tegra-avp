#include <stdbool.h>

#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/timer.h>

#define CNTR_1US 0x00

void udelay(unsigned long delay)
{
	unsigned long base = TEGRA_TMRUS_BASE;
	unsigned long value, start;

	start = readl(base + CNTR_1US);

	while (true) {
		value = readl(base + CNTR_1US);
		if (value > start + delay)
			break;
	}
}
