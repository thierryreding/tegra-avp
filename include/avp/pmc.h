#ifndef AVP_PMC_H
#define AVP_PMC_H

enum pmc_reset_mode {
	PMC_RESET_MODE_RECOVERY,
	PMC_RESET_MODE_BOOTLOADER,
	PMC_RESET_MODE_RCM,
};

struct pmc {
	unsigned long base;
};

void pmc_reset(struct pmc *pmc, enum pmc_reset_mode mode);

extern struct pmc pmc;

#endif
