#include <avp/clk-rst.h>
#include <avp/flow.h>
#include <avp/i2c.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pmc.h>
#include <avp/timer.h>
#include <avp/uart.h>

#define PMC_SECURE_SCRATCH34 0x368
#define PMC_SECURE_SCRATCH35 0x36c
#define PMC_SCRATCH118 0x6f8
#define  PMC_SCRATCH118_MTS_SCRATCH_MASK (0xff << 8)
#define  PMC_SCRATCH118_MTS_HANDSHAKE (1 << 0)

static void __naked switch_to_aarch64(void)
{
	asm (
		"mrc	p15, 0, r0, c12, c0, 2\n"
		"orr	r0, r0, #0x03\n"
		"mcr	p15, 0, r0, c12, c0, 2\n"
	);
}

bool start_cpu(uint32_t entry)
{
	uint32_t value;

	flow_set_active_cluster(&flow, FLOW_CLUSTER_ID_G);

	clock_periph_enable(&clk_cpu);

	pmc_power_up(&pmc, PMC_POWERGATE_ID_CRAIL);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_C0NC);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_CE0);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_CE1);

	/* set VDD_CPU to 1.0V on Norrin64 */
	i2c_init(&dvc, 100000);
	i2c_smbus_write_byte_data(&dvc, 0x40, 0x00, 0x3c);
	udelay(10000);

	flow_repair_ram(&flow);

	clock_periph_enable(&clk_csite);

	/*
	 * The AArch32 reset vector points to code that initiates a switch to
	 * AArch64 via a warm reboot.
	 */
	writel((uint32_t)switch_to_aarch64, TEGRA_EVP_BASE + 0x100);

	/* set AArch64 reset vector */
	writel(entry, TEGRA_PMC_BASE + PMC_SECURE_SCRATCH34);
	writel(0, TEGRA_PMC_BASE + PMC_SECURE_SCRATCH35);

	/* setting up for MTS handshake */
	value = readl(TEGRA_PMC_BASE + PMC_SCRATCH118);
	value &= ~PMC_SCRATCH118_MTS_SCRATCH_MASK;
	value |= PMC_SCRATCH118_MTS_HANDSHAKE;
	writel(value, TEGRA_PMC_BASE + PMC_SCRATCH118);

	reset_cpu_deassert(&clk_rst);

	/* wait for MTS to complete handshake */
	while (true) {
		value = readl(TEGRA_PMC_BASE + PMC_SCRATCH118);
		if ((value & PMC_SCRATCH118_MTS_HANDSHAKE) == 0)
			break;
	}

	return true;
}
