#ifndef AVP_PMC_H
#define AVP_PMC_H

enum pmc_powergate_id {
	PMC_POWERGATE_ID_CRAIL,
	PMC_POWERGATE_ID_CE1 = 9,
	PMC_POWERGATE_ID_CE0 = 14,
	PMC_POWERGATE_ID_C0NC = 15,
};

enum pmc_reset_mode {
	PMC_RESET_MODE_RECOVERY,
	PMC_RESET_MODE_BOOTLOADER,
	PMC_RESET_MODE_RCM,
};

struct pmc {
	unsigned long base;
};

void pmc_reset(struct pmc *pmc, enum pmc_reset_mode mode);
int pmc_power_up(struct pmc *pmc, enum pmc_powergate_id id);

extern struct pmc pmc;

#endif
