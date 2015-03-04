#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/timer.h>

#define CCLK_BURST_POLICY 0x020
#define  CCLK_BURST_POLICY_CPU_STATE_RUN (2 << 28)
#define  CCLK_BURST_POLICY_CWAKEUP_FIQ_SOURCE(x) (((x) & 0xf) << 12)
#define  CCLK_BURST_POLICY_CWAKEUP_IRQ_SOURCE(x) (((x) & 0xf) << 8)
#define  CCLK_BURST_POLICY_CWAKEUP_RUN_SOURCE(x) (((x) & 0xf) << 4)
#define  CCLK_BURST_POLICY_CWAKEUP_IDLE_SOURCE(x) (((x) & 0xf) << 0)

#define SUPER_CCLK_DIVIDER 0x024
#define  SUPER_CCLK_DIVIDER_ENABLE (1 << 31)

#define PLLX_BASE 0x0e0
#define  PLLX_BASE_BYPASS (1 << 31)
#define  PLLX_BASE_ENABLE (1 << 30)
#define  PLLX_BASE_LOCK (1 << 27)
#define  PLLX_BASE_DIVP(x) (((x) & 0x0f) << 20)
#define  PLLX_BASE_DIVN(x) (((x) & 0xff) << 8)
#define  PLLX_BASE_DIVM(x) (((x) & 0xff) << 0)

#define PLLX_MISC 0x0e4
#define  PLLX_MISC_LOCK_ENABLE (1 << 18)

#define CLK_SOURCE_MSELECT 0x3b4
#define  CLK_SOURCE_MSELECT_CLK_SRC_MASK (0x7 << 29)
#define  CLK_SOURCE_MSELECT_CLK_DIVISOR_MASK 0xff

#define RST_CPUG_CMPLX_CLR 0x454
#define RST_CPULP_CMPLX_CLR 0x45c
#define  RST_CMPLX_PRESETDBG (1 << 30)
#define  RST_CMPLX_NONCPURESET (1 << 29)
#define  RST_CMPLX_L2RESET (1 << 24)
#define  RST_CMPLX_CXRESET(x) (1 << (20 + (x)))
#define  RST_CMPLX_CORERESET(x) (1 << (16 + (x)))
#define  RST_CMPLX_DBGRESET(x) (1 << (12 + (x)))
#define  RST_CMPLX_CPURESET(x) (1 << (0 + (x)))

#define PLLX_MISC3 0x518
#define  PLLX_MISC3_IDDQ (1 << 3)

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

static void pllx_init(const struct clk_rst *clk_rst)
{
	/* XXX: do not hardcode the oscillator reference frequency */
	unsigned int divm, divn, divp, rate = 700000, ref = 12000;
	unsigned int vco_min = 700000;
	uint32_t value;

	/* Initialize PLLX */
	value = readl(clk_rst->base + PLLX_MISC3);
	value &= ~PLLX_MISC3_IDDQ;
	writel(value, clk_rst->base + PLLX_MISC3);

	/* XXX: need to stall using flow controller? */
	udelay(2);

	vco_min = ((vco_min + ref - 1) / ref) * ref;

	divp = (vco_min + rate - 1) / rate;
	divm = (ref > 19200) ? 2 : 1;
	divn = (rate * divp * divm) / ref;
	divp--;

	value = PLLX_BASE_BYPASS | PLLX_BASE_DIVP(divp) |
		PLLX_BASE_DIVN(divn) | PLLX_BASE_DIVM(divm);
	writel(value, clk_rst->base + PLLX_BASE);

	value = readl(clk_rst->base + PLLX_BASE);
	value &= ~PLLX_BASE_BYPASS;
	writel(value, clk_rst->base + PLLX_BASE);

	value = readl(clk_rst->base + PLLX_MISC);
	value |= PLLX_MISC_LOCK_ENABLE;
	writel(value, clk_rst->base + PLLX_MISC);

	value = readl(clk_rst->base + PLLX_BASE);
	value |= PLLX_BASE_ENABLE;
	writel(value, clk_rst->base + PLLX_BASE);

	/* wait for PLLX to lock */
	while (true) {
		value = readl(clk_rst->base + PLLX_BASE);
		if (value & PLLX_BASE_LOCK)
			break;
	}

	udelay(300);
}

void clock_cpu_setup(const struct clk_rst *clk_rst)
{
	unsigned int divider;
	uint32_t value;

	pllx_init(clk_rst);

	/* switch MSELECT to PLLP */
	value = readl(clk_rst->base + CLK_SOURCE_MSELECT);
	value &= ~CLK_SOURCE_MSELECT_CLK_SRC_MASK;
	writel(value, clk_rst->base + CLK_SOURCE_MSELECT);

	divider = ((clock_get_rate(&clk_pllp) * 2) / 102000) - 2;

	value = readl(clk_rst->base + CLK_SOURCE_MSELECT);
	value &= ~CLK_SOURCE_MSELECT_CLK_DIVISOR_MASK;
	value |= divider;
	writel(value, clk_rst->base + CLK_SOURCE_MSELECT);

	clock_periph_enable(&clk_mselect);

	value = CCLK_BURST_POLICY_CPU_STATE_RUN |
		CCLK_BURST_POLICY_CWAKEUP_FIQ_SOURCE(8) |
		CCLK_BURST_POLICY_CWAKEUP_IRQ_SOURCE(8) |
		CCLK_BURST_POLICY_CWAKEUP_RUN_SOURCE(8) |
		CCLK_BURST_POLICY_CWAKEUP_IDLE_SOURCE(8);
	writel(value, clk_rst->base + CCLK_BURST_POLICY);

	writel(SUPER_CCLK_DIVIDER_ENABLE, clk_rst->base + SUPER_CCLK_DIVIDER);
}

void reset_cpu_deassert(const struct clk_rst *clk_rst)
{
	uint32_t value;

	/* clear non-CPU resets */
	value = RST_CMPLX_NONCPURESET;
	writel(value, clk_rst->base + RST_CPULP_CMPLX_CLR);

	value = RST_CMPLX_NONCPURESET;
	writel(value, clk_rst->base + RST_CPUG_CMPLX_CLR);

	value = RST_CMPLX_PRESETDBG | RST_CMPLX_L2RESET |
		RST_CMPLX_CXRESET(0) | RST_CMPLX_CORERESET(0) |
		RST_CMPLX_DBGRESET(0) | RST_CMPLX_CPURESET(0);
	writel(value, clk_rst->base + RST_CPULP_CMPLX_CLR);

	value = RST_CMPLX_PRESETDBG | RST_CMPLX_L2RESET |
		RST_CMPLX_CXRESET(3) | RST_CMPLX_CXRESET(2) |
		RST_CMPLX_CXRESET(1) | RST_CMPLX_CXRESET(0) |
		RST_CMPLX_CORERESET(3) | RST_CMPLX_CORERESET(2) |
		RST_CMPLX_CORERESET(1) | RST_CMPLX_CORERESET(0) |
		RST_CMPLX_DBGRESET(3) | RST_CMPLX_DBGRESET(2) |
		RST_CMPLX_DBGRESET(1) | RST_CMPLX_DBGRESET(0) |
		RST_CMPLX_CPURESET(3) | RST_CMPLX_CPURESET(2) |
		RST_CMPLX_CPURESET(1) | RST_CMPLX_CPURESET(0);
	writel(value, clk_rst->base + RST_CPUG_CMPLX_CLR);
}
