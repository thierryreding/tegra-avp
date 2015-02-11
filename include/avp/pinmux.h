#ifndef AVP_PINMUX_H
#define AVP_PINMUX_H

struct pinmux {
	unsigned long base;
};

void pinmux_init(struct pinmux *pinmux);

extern struct pinmux pinmux;

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

void pinmux_config_apply(struct pinmux *pinmux,
			 const struct pinmux_config *config);

#define PINMUX_FUNC_UARTD  0x0
#define PINMUX_FUNC_I2C1   0x0
#define PINMUX_FUNC_I2C2   0x0
#define PINMUX_FUNC_I2CPWR 0x0
#define PINMUX_FUNC_SDMMC4 0x0
#define PINMUX_FUNC_SPI4   0x3
#define PINMUX_FUNC_UARTA  0x3

#define PINMUX_PULL_NORMAL 0x0
#define PINMUX_PULL_DOWN   0x1
#define PINMUX_PULL_UP     0x2
#define PINMUX_PULL_RSVD   0x3

#define PINMUX_TRISTATE_NORMAL   0x0
#define PINMUX_TRISTATE_TRISTATE 0x1

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
		.offset = _offset,						\
		.function = PINMUX_FUNC_##_func,				\
		.pull = PINMUX_PULL_##_pull,					\
		.tristate = PINMUX_TRISTATE_##_tristate,			\
		.input = PINMUX_INPUT_##_input,					\
		.open_drain = PINMUX_OD_##_od,					\
		.lock = PINMUX_LOCK_##_lock,					\
		.ioreset = PINMUX_IORESET_DISABLE,				\
	}

#endif
