#include <avp/clk-rst.h>

static unsigned long clock_pllp_get_rate(const struct clock *clk)
{
	return 216000000;
}

static const struct clock_ops pllp_ops = {
	.get_rate = clock_pllp_get_rate,
};

const struct clock clk_pllp = {
	.clk_rst = &clk_rst,
	.ops = &pllp_ops,
};
