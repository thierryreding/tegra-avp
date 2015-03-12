#ifndef AVP_PINMUX_H
#define AVP_PINMUX_H

#if defined(CONFIG_TEGRA124) || defined(CONFIG_TEGRA132)
#include <avp/tegra124/pinmux.h>
#endif

#if defined(CONFIG_TEGRA210)
#include <avp/tegra210/pinmux.h>
#endif

struct pinmux {
	unsigned long base;
};

void pinmux_init(struct pinmux *pinmux);

extern struct pinmux pinmux;

#endif
