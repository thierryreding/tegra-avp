#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pinmux.h>

void pinmux_config_apply(struct pinmux *pinmux,
			 const struct pinmux_config *config)
{
	uint32_t value;

	value = config->ioreset << 8 | config->lock << 7 |
		config->open_drain << 6 | config->input << 5 |
		config->tristate << 4 | config->pull << 2 |
		config->function;
	writel(value, pinmux->base + config->offset);
}

struct pinmux pinmux = {
	.base = TEGRA_APB_MISC_BASE,
};
