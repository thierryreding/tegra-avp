#include <avp/clk-rst.h>
#include <avp/cpu.h>
#include <avp/flow.h>
#include <avp/i2c.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pmc.h>
#include <avp/timer.h>
#include <avp/uart.h>

#define AARCH64_TRAMPOLINE 0

#if AARCH64_TRAMPOLINE
static void __naked switch_to_aarch64(void)
{
	asm (
		"1:\n"
		"ldr	r1, =0x70006000\n"
		"mov	r0, #'C'\n"
		"str	r0, [r1, #0x00]\n"
		"2:\n"
		"ldr	r0, [r1, #0x14]\n"
		"and	r0, r0, #0x60\n"
		"teq	r0, #0x60\n"
		"bne	2b\n"
		"b	1b\n"
	);
}
#endif

bool start_cpu(uint32_t entry)
{
	flow_set_active_cluster(&flow, FLOW_CLUSTER_ID_G);

	/* TODO: implement MSELECT work around */
	if (1) {
		uint32_t value;

		value = readl(0x50060000 + 0x000);
		value &= ~(1 << 25 | 1 << 24);
		value |= 1 << 29 | 1 << 28 | 1 << 27;
		writel(value, 0x50060000 + 0x000);
	}

	/* TODO: enable VDD_CPU */
	if (1) {
		uint32_t value;

		/* UART2_RTS */
		value = readl(0x6000d108);
		value |= 0x04;
		writel(value, 0x6000d108);

		value = readl(0x6000d118);
		value |= 0x04;
		writel(value, 0x6000d118);

		value = readl(0x6000d128);
		value &= ~0x04;
		writel(value, 0x6000d128);
	}

	if (1) {
		uint8_t value;

		i2c_init(&dvc, 100000);

		/* enable GPIO1 */
		i2c_smbus_read_byte_data(&dvc, 0x3c, 0x40, &value);
		value &= ~(1 << 1);
		i2c_smbus_write_byte_data(&dvc, 0x3c, 0x40, value);

		value = (0x1 << 3) | 1;
		i2c_smbus_write_byte_data(&dvc, 0x3c, 0x37, value);

		/* enable GPIO5 */
		i2c_smbus_read_byte_data(&dvc, 0x3c, 0x40, &value);
		value &= ~(1 << 5);
		i2c_smbus_write_byte_data(&dvc, 0x3c, 0x40, value);

		value = (0x1 << 3) | 1;
		i2c_smbus_write_byte_data(&dvc, 0x3c, 0x3b, value);
	}

	clock_cpu_setup(&clk_rst);
	clock_periph_enable(&clk_csite);

	if (1) {
		uint32_t value;

		value = readl(0x60006000 + 0x388);
		value &= ~0xfff;
		writel(value, 0x60006000 + 0x388);
	}

	pmc_power_up(&pmc, PMC_POWERGATE_ID_CRAIL);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_C0NC);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_CE0);

	flow_repair_ram(&flow);

#if AARCH64_TRAMPOLINE
	writel((uint32_t)switch_to_aarch64, TEGRA_EVP_BASE + 0x100);
#else
	writel(entry | 0x1, TEGRA_EVP_BASE + 0x100);
#endif

	/* set the AArch64 warm reset vector */
	writel(entry | 0x1, 0x6000c230);
	writel(0, 0x6000c234);

	//reset_deassert(&rst_mselect);
	reset_cpu_deassert(&clk_rst);

	return true;
}
