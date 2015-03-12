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

void clock_pllm_init(const struct clk_rst *clk_rst,
		     const struct bct_sdram_params *params)
{
	uint32_t value;

	value = readl(clk_rst->base + PLLM_OUT);
	value &= ~PLLM_OUT_RESET_DISABLE;
	writel(value, clk_rst->base + PLLM_OUT);

	value = PLLM_MISC1_PD_LSHIFT_PH135(params->pll_m_pd_lshift_ph135) |
		PLLM_MISC1_PD_LSHIFT_PH90(params->pll_m_pd_lshift_ph90) |
		PLLM_MISC1_PD_LSHIFT_PH45(params->pll_m_pd_lshift_ph45) |
		PLLM_MISC1_SETUP(params->pll_m_setup_control);
	writel(value, clk_rst->base + PLLM_MISC1);

	value = PLLM_MISC2_KCP(params->pll_m_kcp) |
		PLLM_MISC2_KVCO(params->pll_m_kvco);
	writel(value, clk_rst->base + PLLM_MISC2);

	value = PLLM_BASE_DIV2(params->pll_m_select_div2) |
		PLLM_BASE_DIVN(params->pll_m_div_n) |
		PLLM_BASE_DIVM(params->pll_m_div_m);
	writel(value, clk_rst->base + PLLM_BASE);

	value = readl(clk_rst->base + PLLM_BASE);
	value |= PLLM_BASE_ENABLE;
	writel(value, clk_rst->base + PLLM_BASE);

	while (true) {
		value = readl(clk_rst->base + PLLM_BASE);
		if (value & PLLM_BASE_LOCK)
			break;
	}

	/* stabilization delay */
	udelay(10);

	value = readl(clk_rst->base + PLLM_OUT);
	value |= PLLM_OUT_RESET_DISABLE;
	writel(value, clk_rst->base + PLLM_OUT);

	value = readl(clk_rst->base + CLK_SOURCE_EMC);
	value &= ~CLK_SOURCE_EMC_2X_CLK_SRC_MASK;
	value |= params->emc_clock_source;

	if ((params->mc_emem_arb_misc0 & CLK_SOURCE_EMC_MC_SAME_FREQ) == 0)
		value &= ~CLK_SOURCE_EMC_MC_SAME_FREQ;
	else
		value |= CLK_SOURCE_EMC_MC_SAME_FREQ;

	writel(value, clk_rst->base + CLK_SOURCE_EMC);

	clock_periph_enable(&clk_emc);
	clock_periph_enable(&clk_mc);
}

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
