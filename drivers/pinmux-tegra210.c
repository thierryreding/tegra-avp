#include <common.h>
#include <types.h>

#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pinmux.h>

#define PINMUX_GLOBAL_0 0x0040
#define  PINMUX_GLOBAL_CLAMP_TRISTATE_INPUTS (1 << 0)

#define PINMUX_AUX_GEN1_I2C_SCL 0x30bc
#define PINMUX_AUX_GEN1_I2C_SDA 0x30c0
#define PINMUX_AUX_GEN2_I2C_SCL 0x30c4
#define PINMUX_AUX_GEN2_I2C_SDA 0x30c8
#define PINMUX_AUX_PWR_I2C_SCL  0x30dc
#define PINMUX_AUX_PWR_I2C_SDA  0x30e0
#define PINMUX_AUX_UART1_TX     0x30e4
#define PINMUX_AUX_UART1_RX     0x30e8
#define PINMUX_AUX_UART1_RTS    0x30ec
#define PINMUX_AUX_UART1_CTS    0x30f0

static const struct pinmux_config pinmux_configs[] = {
	PINMUX_CONFIG(GEN1_I2C_SCL, I2C1,   NONE, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GEN1_I2C_SDA, I2C1,   NONE, NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(GEN2_I2C_SCL, I2C2,   NONE, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(GEN2_I2C_SDA, I2C2,   NONE, NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(PWR_I2C_SCL,  I2CPWR, NONE, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(PWR_I2C_SDA,  I2CPWR, NONE, NORMAL, ENABLE,  DISABLE, DISABLE),

	PINMUX_CONFIG(UART1_RTS,    UARTA,  NONE, NORMAL, DISABLE, DISABLE, DISABLE),
	PINMUX_CONFIG(UART1_CTS,    UARTA,  NONE, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(UART1_TX,     UARTA,  NONE, NORMAL, ENABLE,  DISABLE, DISABLE),
	PINMUX_CONFIG(UART1_RX,     UARTA,  NONE, NORMAL, ENABLE,  DISABLE, DISABLE),
};

static void pinmux_clamp_enable(struct pinmux *pinmux)
{
	uint32_t value;

	value = readl(pinmux->base + PINMUX_GLOBAL_0);
	value &= ~PINMUX_GLOBAL_CLAMP_TRISTATE_INPUTS;
	writel(value, pinmux->base + PINMUX_GLOBAL_0);
}

static void pinmux_config_apply(struct pinmux *pinmux,
				const struct pinmux_config *config)
{
	uint32_t value;

	value = config->ioreset << 16 | config->open_drain << 11 |
		config->lock << 7 | config->input << 6 |
		config->tristate << 4 | config->pull << 2 |
		config->function;
	writel(value, pinmux->base + config->offset);
}

void pinmux_init(struct pinmux *pinmux)
{
	unsigned int i;

	pinmux_clamp_enable(pinmux);

	for (i = 0; i < ARRAY_SIZE(pinmux_configs); i++)
		pinmux_config_apply(pinmux, &pinmux_configs[i]);
}

struct pinmux pinmux = {
	.base = TEGRA_APB_MISC_BASE,
};
