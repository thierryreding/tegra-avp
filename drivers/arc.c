#include <types.h>

#include <avp/io.h>
#include <avp/iomap.h>

#define CLK_RST_LVL2_CLK_GATE_OVRD 0x3a4
#define  LVL2_CLK_GATE_OVRD_ARC (1 << 19)

#define MC_IRAM_BOM 0x65c
#define MC_IRAM_TOM 0x660

#define MC_IRAM_REG_CTRL 0x964
#define  MC_IRAM_REG_CTRL_DISABLE (1 << 0)

void arc_enable(void)
{
	unsigned long base;
	uint32_t value;

	base = TEGRA_CLK_RST_BASE;

	value = readl(base + CLK_RST_LVL2_CLK_GATE_OVRD);
	value |= LVL2_CLK_GATE_OVRD_ARC;
	writel(value, base + CLK_RST_LVL2_CLK_GATE_OVRD);

	base = TEGRA_MC_BASE;

	value = readl(base + MC_IRAM_REG_CTRL);
	value &= ~MC_IRAM_REG_CTRL_DISABLE;
	writel(value, base + MC_IRAM_REG_CTRL);

	writel(TEGRA_IRAM_BASE, base + MC_IRAM_BOM);
	writel(TEGRA_IRAM_BASE + TEGRA_IRAM_SIZE, base + MC_IRAM_TOM);

	/* flush writes */
	readl(base + MC_IRAM_REG_CTRL);
}
