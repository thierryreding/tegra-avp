#ifndef AVP_CLK_RST_H
#define AVP_CLK_RST_H

struct clk;

struct clk_rst {
	unsigned long base;
};

struct clk_ops {
	unsigned long (*get_rate)(const struct clk *clk);
};

struct clk {
	const struct clk_rst *clk_rst;
	const struct clk_ops *ops;
};

unsigned long clk_get_rate(const struct clk *clk);

struct clk_periph {
	struct clk base;

	unsigned int set;
	unsigned int clr;
	unsigned int bit;

	unsigned int src;
};

void clk_periph_enable(const struct clk_periph *clk);
void clk_periph_disable(const struct clk_periph *clk);
void clk_periph_set_source(const struct clk_periph *clk,
			   unsigned int source);

struct reset {
	const struct clk_rst *clk_rst;

	unsigned int set;
	unsigned int clr;
	unsigned int bit;
};

void reset_assert(const struct reset *rst);
void reset_deassert(const struct reset *rst);

extern const struct clk_rst clk_rst;

extern const struct clk clk_pllp;

extern const struct clk_periph clk_usbd;
extern const struct clk_periph clk_uartd;

extern const struct reset rst_usbd;
extern const struct reset rst_uartd;

#endif
