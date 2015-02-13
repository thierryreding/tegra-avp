#include <avp/clk-rst.h>

static unsigned long clk_pllp_get_rate(const struct clk *clk)
{
	return 216000000;
}

static const struct clk_ops pllp_ops = {
	.get_rate = clk_pllp_get_rate,
};

const struct clk clk_pllp = {
	.clk_rst = &clk_rst,
	.ops = &pllp_ops,
};
