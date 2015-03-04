#include <avp/clk-rst.h>
#include <avp/flow.h>
#include <avp/i2c.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pmc.h>
#include <avp/timer.h>
#include <avp/uart.h>

bool start_cpu(uint32_t entry)
{
	/*
	 * On 32-bit Tegra the boot procedure is to jump to U-Boot SPL, which
	 * will continue to run on the AVP. In those cases we do not want to
	 * initialize the CPU. Returning false here will cause recovery mode
	 * to have AVP jump to the bootloader.
	 */
	return false;

	flow_set_active_cluster(&flow, FLOW_CLUSTER_ID_G);

	/* set VDD_CPU to 1.0V on Jetson TK1 */
	i2c_init(&dvc, 100000);
	i2c_smbus_write_byte_data(&dvc, 0x40, 0x00, 0x3c);
	udelay(10000);

	flow_repair_ram(&flow);

	clock_cpu_setup(&clk_rst);

	clock_periph_enable(&clk_cpu);
	clock_periph_enable(&clk_cpulp);
	clock_periph_enable(&clk_cpug);

	clock_periph_enable(&clk_csite);

	reset_deassert(&rst_mselect);
	reset_cpu_deassert(&clk_rst);

	writel(entry, TEGRA_EVP_BASE + 0x100);

	pmc_power_up(&pmc, PMC_POWERGATE_ID_CRAIL);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_C0NC);
	pmc_power_up(&pmc, PMC_POWERGATE_ID_CE0);

	return true;
}
