#ifndef AVP_I2C_H
#define AVP_I2C_H

#include <types.h>

struct clock;
struct reset;

struct i2c {
	unsigned long base;
	unsigned int speed;
	const struct clock_periph *clk;
	const struct reset *rst;
};

int i2c_init(struct i2c *i2c, unsigned int speed);
int i2c_smbus_write_byte_data(struct i2c *i2c, uint8_t address,
			      uint8_t command, uint8_t value);
int i2c_smbus_read_byte_data(struct i2c *i2c, uint8_t address,
			     uint8_t command, uint8_t *valuep);

extern struct i2c dvc;

#endif
