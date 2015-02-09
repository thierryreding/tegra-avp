#ifndef AVP_PINMUX_H
#define AVP_PINMUX_H

struct pinmux {
	unsigned long base;
};

void pinmux_init(struct pinmux *pinmux, unsigned long base);

struct pinmux_config {
	unsigned int reg;
	unsigned int func;
	unsigned int pull;
	unsigned int tristate;
	unsigned int input;
};

void pinmux_config_apply(struct pinmux *pinmux,
			 const struct pinmux_config *config);

#define PINMUX_FUNC_UARTD  0x0

#define PINMUX_PULL_NORMAL 0x0
#define PINMUX_PULL_DOWN   0x1
#define PINMUX_PULL_UP     0x2
#define PINMUX_PULL_RSVD   0x3

#define PINMUX_TRISTATE_NORMAL   0x0
#define PINMUX_TRISTATE_TRISTATE 0x1

#define PINMUX_INPUT_DISABLE 0x0
#define PINMUX_INPUT_ENABLE  0x1

#define PINMUX_CONFIG(_reg, _func, _pull, _tristate, _input)	\
	{							\
		.reg = _reg,					\
		.func = PINMUX_FUNC_##_func,			\
		.pull = PINMUX_PULL_##_pull,			\
		.tristate = PINMUX_TRISTATE_##_tristate,	\
		.input = PINMUX_INPUT_##_input,			\
	}

#endif
