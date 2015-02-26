#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>

unsigned long clk_get_rate(const struct clk *clk)
{
	return clk->ops->get_rate(clk);
}

void clk_periph_enable(const struct clk_periph *clk)
{
	writel(1 << clk->bit, clk->base.clk_rst->base + clk->set);
}

void clk_periph_disable(const struct clk_periph *clk)
{
	writel(1 << clk->bit, clk->base.clk_rst->base + clk->clr);
}

void clk_periph_set_source(const struct clk_periph *clk, unsigned int source)
{
	unsigned long value;

	value = readl(clk->base.clk_rst->base + clk->src);
	value &= ~(0x7 << 29);
	writel(value, clk->base.clk_rst->base + clk->src);
}

void reset_assert(const struct reset *rst)
{
	writel(1 << rst->bit, rst->clk_rst->base + rst->set);
}

void reset_deassert(const struct reset *rst)
{
	writel(1 << rst->bit, rst->clk_rst->base + rst->clr);
}

const struct clk_rst clk_rst = {
	.base = TEGRA_CLK_RST_BASE,
};

const struct clk_periph clk_usbd = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x320,
	.clr = 0x324,
	.bit = 22,
	.src = 0x000,
};

const struct clk_periph clk_mc = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x328,
	.clr = 0x32c,
	.bit = 0,
	.src = 0x000,
};

const struct clk_periph clk_emc = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x328,
	.clr = 0x32c,
	.bit = 25,
	.src = 0x000,
};

const struct clk_periph clk_uartd = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x330,
	.clr = 0x334,
	.bit = 1,
	.src = 0x1c0,
};

const struct reset rst_usbd = {
	.clk_rst = &clk_rst,
	.set = 0x300,
	.clr = 0x304,
	.bit = 22,
};

const struct reset rst_mc = {
	.clk_rst = &clk_rst,
	.set = 0x308,
	.clr = 0x30c,
	.bit = 0,
};

const struct reset rst_emc = {
	.clk_rst = &clk_rst,
	.set = 0x308,
	.clr = 0x30c,
	.bit = 25,
};

const struct reset rst_uartd = {
	.clk_rst = &clk_rst,
	.set = 0x310,
	.clr = 0x314,
	.bit = 1,
};
