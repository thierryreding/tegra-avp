#include <types.h>

#include <avp/clk-rst.h>
#include <avp/i2c.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/timer.h>
#include <avp/uart.h>

#define I2C_CNFG      0x000
#define  I2C_CNFG_CMD1  (1 << 6)
#define  I2C_CNFG_START (1 << 5)
#define I2C_CMD_ADDR0 0x004
#define I2C_CMD_DATA0 0x00c
#define I2C_CLK_DIV 0x06c

int i2c_init(struct i2c *i2c, unsigned int speed)
{
	unsigned int divisor;

	reset_assert(i2c->rst);
	clock_periph_enable(i2c->clk);

	divisor = clock_get_rate(&clk_pllp) / (8 * 0x1a * speed * 2);
	clock_periph_set_source_divisor(i2c->clk, 0, divisor);

	reset_deassert(i2c->rst);

	if (1) {
		uint32_t value = readl(i2c->base + I2C_CLK_DIV);
		uart_printf(debug, "clock divider: %08x\n", value);
	}

	return 0;
}

int i2c_smbus_write_byte_data(struct i2c *i2c, uint8_t address,
			      uint8_t command, uint8_t value)
{
	writel(address << 1, i2c->base + I2C_CMD_ADDR0);
	writel(0x0002, i2c->base + I2C_CNFG);

	writel(value << 8 | command, i2c->base + I2C_CMD_DATA0);
	writel(0x0a02, i2c->base + I2C_CNFG);
	//writel(I2C_CNFG_CMD1 | I2C_CNFG_START | 0x02, i2c->base + I2C_CNFG);

	return 0;
}

struct i2c dvc = {
	.base = 0x7000d000,
	.speed = 400000,
	.clk = &clk_i2c5,
	.rst = &rst_i2c5,
};
