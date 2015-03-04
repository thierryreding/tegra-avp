#include <avp/clk-rst.h>
#include <avp/io.h>

#define RST_CPU_CMPLX_SET 0x340

#define RST_CPUG_CMPLX_CLR 0x454
#define  RST_CMPLX_PRESETDBG (1 << 30)
#define  RST_CMPLX_NONCPURESET (1 << 29)
#define  RST_CMPLX_L2RESET (1 << 24)
#define  RST_CMPLX_CXRESET(x) (1 << (20 + (x)))
#define  RST_CMPLX_CORERESET(x) (1 << (16 + (x)))
#define  RST_CMPLX_DBGRESET(x) (1 << (12 + (x)))
#define  RST_CMPLX_CPURESET(x) (1 << (0 + (x)))

static unsigned long clock_pllp_get_rate(const struct clock *clk)
{
	return 408000000;
}

static const struct clock_ops pllp_ops = {
	.get_rate = clock_pllp_get_rate,
};

const struct clock clk_pllp = {
	.clk_rst = &clk_rst,
	.ops = &pllp_ops,
};

void reset_cpu_deassert(const struct clk_rst *clk_rst)
{
	uint32_t value;

	value = readl(clk_rst->base + RST_CPU_CMPLX_SET);
	value |= RST_CMPLX_CXRESET(1);
	writel(value, clk_rst->base + RST_CPU_CMPLX_SET);

	value = RST_CMPLX_NONCPURESET;
	writel(value, clk_rst->base + RST_CPUG_CMPLX_CLR);

	value = RST_CMPLX_PRESETDBG | RST_CMPLX_L2RESET |
		RST_CMPLX_CXRESET(0) | RST_CMPLX_CORERESET(0) |
		RST_CMPLX_DBGRESET(0) | RST_CMPLX_CPURESET(0);
	writel(value, clk_rst->base + RST_CPUG_CMPLX_CLR);
}
