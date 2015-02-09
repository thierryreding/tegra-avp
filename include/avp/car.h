#ifndef AVP_CAR_H
#define AVP_CAR_H

struct car {
	unsigned long base;
};

void car_init(struct car *car, unsigned long base);

struct clk_pll {
	unsigned int base;
	unsigned int misc;
};

struct clk_periph {
	unsigned int set;
	unsigned int clr;
	unsigned int bit;

	unsigned int src;
};

void clk_periph_enable(struct car *car, const struct clk_periph *clk);
void clk_periph_disable(struct car *car, const struct clk_periph *clk);
void clk_periph_set_source(struct car *car, const struct clk_periph *clk,
			   unsigned int source);

const struct clk_periph clk_uartd;

struct reset {
	unsigned int set;
	unsigned int clr;
	unsigned int bit;
};

const struct reset rst_uartd;

void reset_assert(struct car *car, const struct reset *rst);
void reset_deassert(struct car *car, const struct reset *rst);

#endif
