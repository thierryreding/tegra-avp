#ifndef AVP_TEGRA210_PINMUX_H
#define AVP_TEGRA210_PINMUX_H

struct pinmux_config {
	unsigned int offset;
	unsigned int function;
	unsigned int pull;
	unsigned int tristate;
	unsigned int input;
	unsigned int open_drain;
	unsigned int lock;
	unsigned int ioreset;
};

#define PINMUX_FUNC_I2C1   0x0
#define PINMUX_FUNC_I2C2   0x0
#define PINMUX_FUNC_I2CPWR 0x0
#define PINMUX_FUNC_UARTA  0x0

#define PINMUX_PULL_NONE 0x0

#define PINMUX_TRISTATE_NORMAL 0x0

#define PINMUX_INPUT_DISABLE 0x0
#define PINMUX_INPUT_ENABLE  0x1

#define PINMUX_OD_DISABLE 0x0
#define PINMUX_OD_ENABLE  0x1

#define PINMUX_LOCK_DISABLE 0x0
#define PINMUX_LOCK_ENABLE  0x1

#define PINMUX_IORESET_DISABLE 0x0
#define PINMUX_IORESET_ENABLE  0x1

#define PINMUX_CONFIG(_offset, _func, _pull, _tristate, _input, _od, _lock)	\
	{									\
		.offset = PINMUX_AUX_##_offset,					\
		.function = PINMUX_FUNC_##_func,				\
		.pull = PINMUX_PULL_##_pull,					\
		.tristate = PINMUX_TRISTATE_##_tristate,			\
		.input = PINMUX_INPUT_##_input,					\
		.open_drain = PINMUX_OD_##_od,					\
		.lock = PINMUX_LOCK_##_lock,					\
		.ioreset = PINMUX_IORESET_DISABLE,				\
	}

#endif
