#ifndef AVP_CLK_RST_H
#define AVP_CLK_RST_H

struct bct_sdram_params;
struct clock;

enum osc_freq {
	OSC_FREQ_13,
	OSC_FREQ_16_8,
	OSC_FREQ_19_2 = 4,
	OSC_FREQ_38_4,
	OSC_FREQ_12 = 8,
	OSC_FREQ_48,
	OSC_FREQ_26 = 12,
};

struct clk_rst {
	unsigned long base;
};

void clock_osc_init(const struct clk_rst *clk_rst);
void clock_pllm_init(const struct clk_rst *clk_rst,
		     const struct bct_sdram_params *params);

struct clock_ops {
	unsigned long (*get_rate)(const struct clock *clk);
};

struct clock {
	const struct clk_rst *clk_rst;
	const struct clock_ops *ops;
};

unsigned long clock_get_rate(const struct clock *clock);

struct clock_periph {
	struct clock base;

	unsigned int set;
	unsigned int clr;
	unsigned int bit;

	unsigned int src;
};

void clock_periph_enable(const struct clock_periph *clk);
void clock_periph_disable(const struct clock_periph *clk);
void clock_periph_set_source(const struct clock_periph *clk,
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

extern const struct clock clk_pllp;

extern const struct clock_periph clk_usbd;
extern const struct clock_periph clk_mc;
extern const struct clock_periph clk_emc;
extern const struct clock_periph clk_uarta;
extern const struct clock_periph clk_uartd;

extern const struct reset rst_usbd;
extern const struct reset rst_mc;
extern const struct reset rst_emc;
extern const struct reset rst_uarta;
extern const struct reset rst_uartd;

#endif
