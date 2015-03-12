#include <avp/board.h>
#include <avp/i2c.h>

#define PMIC_ADDRESS 0x3c

#define ON_OFF_CFG1 0x41
#define  ON_OFF_CFG1_PWR_OFF (1 << 1)

static void loki_power_off(void)
{
	uint8_t value;

	i2c_smbus_read_byte_data(&dvc, PMIC_ADDRESS, ON_OFF_CFG1, &value);
	value |= ON_OFF_CFG1_PWR_OFF;
	i2c_smbus_write_byte_data(&dvc, PMIC_ADDRESS, ON_OFF_CFG1, value);
}

static struct board loki = {
	.power_off = loki_power_off,
};
board_init(loki);
