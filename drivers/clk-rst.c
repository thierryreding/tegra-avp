#include <avp/bct.h>
#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/timer.h>

#define SYSTEM_RATE 0x030
#define  SYSTEM_RATE_HCLK_DISABLE (1 << 7)
#define  SYSTEM_RATE_AHB(x) (((x) & 0x3) << 4)
#define  SYSTEM_RATE_PCLK_DISABLE (1 << 3)
#define  SYSTEM_RATE_APB(x) (((x) & 0x3) << 0)

#define OSC_CTRL 0x050
#define  OSC_CTRL_FREQ(x) (((x) & 0xf) << 28)
#define  OSC_CTRL_PLL_REF_DIV4 (2 << 26)
#define  OSC_CTRL_PLL_REF_DIV2 (1 << 26)
#define  OSC_CTRL_PLL_REF_DIV1 (0 << 26)
#define  OSC_CTRL_SPARE(x) (((x) & 0xff) << 18)
#define  OSC_CTRL_DUTY_CYCLE(x) (((x) & 0x1f) << 12)
#define  OSC_CTRL_DRIVE_STRENGTH(x) (((x) & 0x3f) << 4)
#define  OSC_CTRL_ENABLE (1 << 0)

#define OSC_FREQ_DET 0x058
#define  OSC_FREQ_DET_ENABLE (1 << 31)
#define  OSC_FREQ_DET_REF_CLK_WIN_CFG(x) (((x) - 1) & 0xf)

#define OSC_FREQ_DET_STATUS 0x05c
#define  OSC_FREQ_DET_BUSY (1 << 31)
#define  OSC_FREQ_DET_CNT_MASK 0xffff

#define PLLM_BASE 0x090
#define  PLLM_BASE_ENABLE (1 << 30)
#define  PLLM_BASE_LOCK (1 << 27)
#define  PLLM_BASE_DIV2(x) (((x) & 0x1) << 20)
#define  PLLM_BASE_DIVN(x) (((x) & 0xff) << 8)
#define  PLLM_BASE_DIVM(x) (((x) & 0xff) << 0)

#define PLLM_OUT 0x094
#define  PLLM_OUT_RESET_DISABLE (1 << 0)

#define PLLM_MISC1 0x098
#define  PLLM_MISC1_PD_LSHIFT_PH135(x) (((x) & 0x1) << 30)
#define  PLLM_MISC1_PD_LSHIFT_PH90(x) (((x) & 0x1) << 29)
#define  PLLM_MISC1_PD_LSHIFT_PH45(x) (((x) & 0x1) << 28)
#define  PLLM_MISC1_SETUP(x) (((x) & 0xffffff) << 0)

#define PLLM_MISC2 0x09c
#define  PLLM_MISC2_KCP(x) (((x) & 0x3) << 1)
#define  PLLM_MISC2_KVCO(x) (((x) & 0x1) << 0)

#define CLK_SOURCE_EMC 0x19c
#define  CLK_SOURCE_EMC_2X_CLK_SRC_MASK (0x7 << 29)
#define  CLK_SOURCE_EMC_MC_SAME_FREQ (1 << 16)

struct osc_freq_entry {
	enum osc_freq freq;
	unsigned int min;
	unsigned int max;
};

#define OSC_FREQ_MIN(f) (((f) - ((f) >> 5) - (1 << 15) + 1) >> 15)
#define OSC_FREQ_MAX(f) (((f) + ((f) >> 5) + (1 << 15) - 1) >> 15)

static const struct osc_freq_entry osc_freq_table[] = {
	{ OSC_FREQ_13,   OSC_FREQ_MIN(13000000), OSC_FREQ_MAX(13000000) },
	{ OSC_FREQ_19_2, OSC_FREQ_MIN(19200000), OSC_FREQ_MAX(19200000) },
	{ OSC_FREQ_12,   OSC_FREQ_MIN(12000000), OSC_FREQ_MAX(12000000) },
	{ OSC_FREQ_26,   OSC_FREQ_MIN(26000000), OSC_FREQ_MAX(26000000) },
	{ OSC_FREQ_16_8, OSC_FREQ_MIN(16800000), OSC_FREQ_MAX(16800000) },
	{ OSC_FREQ_38_4, OSC_FREQ_MIN(38400000), OSC_FREQ_MAX(38400000) },
	{ OSC_FREQ_48,   OSC_FREQ_MIN(48000000), OSC_FREQ_MAX(48000000) },
};

void clock_osc_init(const struct clk_rst *clk_rst)
{
	unsigned int i, pll_ref_div;
	uint32_t value;
	int freq = -1;

	value = readl(clk_rst->base + OSC_FREQ_DET);
	value &= ~OSC_FREQ_DET_REF_CLK_WIN_CFG(0xf);
	value |= OSC_FREQ_DET_REF_CLK_WIN_CFG(1);
	value |= OSC_FREQ_DET_ENABLE;
	writel(value, clk_rst->base + OSC_FREQ_DET);

	while (true) {
		value = readl(clk_rst->base + OSC_FREQ_DET_STATUS);
		if ((value & OSC_FREQ_DET_BUSY) == 0)
			break;
	}

	value &= OSC_FREQ_DET_CNT_MASK;

	for (i = 0; i < ARRAY_SIZE(osc_freq_table); i++) {
		if ((value >= osc_freq_table[i].min) &&
		    (value <= osc_freq_table[i].max)) {
			freq = osc_freq_table[i].freq;
			break;
		}
	}

	if (freq < 0) {
		uart_printf(debug, "WARNING: unknown oscillator frequency\n");
		freq = OSC_FREQ_12;
	}

	timer_us_init(freq);

	switch (freq) {
	case OSC_FREQ_13:
	case OSC_FREQ_16_8:
	case OSC_FREQ_19_2:
	case OSC_FREQ_12:
	case OSC_FREQ_26:
	default:
		pll_ref_div = OSC_CTRL_PLL_REF_DIV1;
		break;

	case OSC_FREQ_38_4:
		pll_ref_div = OSC_CTRL_PLL_REF_DIV2;
		break;

	case OSC_FREQ_48:
		pll_ref_div = OSC_CTRL_PLL_REF_DIV4;
		break;
	}

	value = OSC_CTRL_FREQ(freq) | pll_ref_div | OSC_CTRL_SPARE(0) |
		OSC_CTRL_DUTY_CYCLE(0) | OSC_CTRL_DRIVE_STRENGTH(1) |
		OSC_CTRL_ENABLE;
	writel(value, clk_rst->base + OSC_CTRL);

	value = SYSTEM_RATE_AHB(1) | SYSTEM_RATE_APB(0);
	writel(value, clk_rst->base + SYSTEM_RATE);
}

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
}

unsigned long clock_get_rate(const struct clock *clk)
{
	return clk->ops->get_rate(clk);
}

void clock_periph_enable(const struct clock_periph *clk)
{
	writel(1 << clk->bit, clk->base.clk_rst->base + clk->set);
}

void clock_periph_disable(const struct clock_periph *clk)
{
	writel(1 << clk->bit, clk->base.clk_rst->base + clk->clr);
}

void clock_periph_set_source_divisor(const struct clock_periph *clk,
				     unsigned int source,
				     unsigned int divisor)
{
	unsigned long value;

	value = readl(clk->base.clk_rst->base + clk->src);
	value &= ~(0x7 << 29);
	value |= source << 29;
	value &= ~0xffff;
	value |= divisor & 0xffff;
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

const struct clock_periph clk_cpu = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x320,
	.clr = 0x324,
	.bit = 0,
};

const struct clock_periph clk_uarta = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x320,
	.clr = 0x324,
	.bit = 6,
	.src = 0x178,
};

const struct clock_periph clk_usbd = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x320,
	.clr = 0x324,
	.bit = 22,
	.src = 0x000,
};

const struct clock_periph clk_mc = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x328,
	.clr = 0x32c,
	.bit = 0,
	.src = 0x000,
};

const struct clock_periph clk_i2c5 = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x328,
	.clr = 0x32c,
	.bit = 15,
	.src = 0x128,
};

const struct clock_periph clk_emc = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x328,
	.clr = 0x32c,
	.bit = 25,
	.src = 0x19c,
};

const struct clock_periph clk_uartd = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x330,
	.clr = 0x334,
	.bit = 1,
	.src = 0x1c0,
};

const struct clock_periph clk_csite = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x330,
	.clr = 0x334,
	.bit = 9,
};

const struct clock_periph clk_cpug = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x440,
	.clr = 0x444,
	.bit = 0,
};

const struct clock_periph clk_cpulp = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x440,
	.clr = 0x444,
	.bit = 1,
};

const struct clock_periph clk_mselect = {
	.base = {
		.clk_rst = &clk_rst,
	},
	.set = 0x440,
	.clr = 0x444,
	.bit = 3,
};

const struct reset rst_cpu = {
	.clk_rst = &clk_rst,
	.set = 0x300,
	.clr = 0x304,
	.bit = 0,
};

const struct reset rst_uarta = {
	.clk_rst = &clk_rst,
	.set = 0x300,
	.clr = 0x304,
	.bit = 6,
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

const struct reset rst_i2c5 = {
	.clk_rst = &clk_rst,
	.set = 0x308,
	.clr = 0x30c,
	.bit = 15,
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

const struct reset rst_mselect = {
	.clk_rst = &clk_rst,
	.set = 0x430,
	.clr = 0x434,
	.bit = 3,
};
