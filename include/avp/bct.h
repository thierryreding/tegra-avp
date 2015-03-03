#ifndef AVP_BCT_H
#define AVP_BCT_H

#ifdef CONFIG_TEGRA114
#include <avp/tegra114/bct.h>
#endif

#if defined(CONFIG_TEGRA124) || defined(CONFIG_TEGRA132)
#include <avp/tegra124/bct.h>
#endif

#endif
