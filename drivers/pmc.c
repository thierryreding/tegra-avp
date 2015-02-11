#include <avp/io.h>
#include <avp/pmc.h>

#define PMC_CNTRL 0x000
#define  PMC_CNTRL_MAIN_RST (1 << 4)

#define PMC_SCRATCH0 0x050
#define  PMC_SCRATCH0_MODE_RECOVERY (1 << 31)
#define  PMC_SCRATCH0_MODE_BOOTLOADER (1 << 30)
#define  PMC_SCRATCH0_MODE_RCM (1 << 1)

void pmc_reset(struct pmc *pmc, enum pmc_reset_mode mode)
{
	unsigned long value;

	value = readl(pmc->base + PMC_SCRATCH0);

	switch (mode) {
	case PMC_RESET_MODE_RECOVERY:
		value |= PMC_SCRATCH0_MODE_RECOVERY;
		break;

	case PMC_RESET_MODE_BOOTLOADER:
		value |= PMC_SCRATCH0_MODE_BOOTLOADER;
		break;

	case PMC_RESET_MODE_RCM:
		value |= PMC_SCRATCH0_MODE_RCM;
		break;
	}

	writel(value, pmc->base + PMC_SCRATCH0);

	value = readl(pmc->base + PMC_CNTRL);
	value |= PMC_CNTRL_MAIN_RST;
	writel(value, pmc->base + PMC_CNTRL);
}

struct pmc pmc = {
	.base = 0x7000e400,
};
