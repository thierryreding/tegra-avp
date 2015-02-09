#include <avp/car.h>
#include <avp/io.h>

const struct clk_pll pll_p = {
	.base = 0x0a0,
	.misc = 0x0ac,
};

const struct clk_pll pll_c = {
	.base = 0,
	.misc = 0,
};

const struct clk_periph clk_uartd = {
	.set = 0x330,
	.clr = 0x334,
	.bit = 0,
	.src = 0x1c0,
};

const struct reset rst_uartd = {
	.set = 0x310,
	.clr = 0x314,
	.bit = 0,
};

void car_init(struct car *car, unsigned long base)
{
	car->base = base;
}

void clk_periph_enable(struct car *car, const struct clk_periph *clk)
{
	writel(1 << clk->bit, car->base + clk->set);
}

void clk_periph_disable(struct car *car, const struct clk_periph *clk)
{
	writel(1 << clk->bit, car->base + clk->clr);
}

void clk_periph_set_source(struct car *car, const struct clk_periph *clk,
			   unsigned int source)
{
	unsigned long value;

	value = readl(car->base + clk->src);
	value &= ~(0x7 << 29);
	writel(value, car->base + clk->src);
}

void reset_assert(struct car *car, const struct reset *rst)
{
	writel(1 << rst->bit, car->base + rst->set);
}

void reset_deassert(struct car *car, const struct reset *rst)
{
	writel(1 << rst->bit, car->base + rst->clr);
}
