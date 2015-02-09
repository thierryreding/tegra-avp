#include <avp/io.h>
#include <avp/pinmux.h>

void pinmux_init(struct pinmux *pinmux, unsigned long base)
{
	pinmux->base = base;
}

void pinmux_config_apply(struct pinmux *pinmux,
			 const struct pinmux_config *config)
{
	unsigned long value;

	value = readl(pinmux->base + config->reg);

	value &= ~0x3;
	value |= config->func;

	value &= ~(0x3 << 2);
	value |= config->pull << 2;

	if (config->tristate == PINMUX_TRISTATE_NORMAL)
		value &= ~(1 << 4);
	else
		value |= 1 << 4;

	if (config->input == PINMUX_INPUT_DISABLE)
		value &= ~(1 << 5);
	else
		value |= 1 << 5;

	writel(value, pinmux->base + config->reg);
}

