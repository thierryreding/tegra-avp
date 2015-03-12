#include <ctype.h>
#include <nv3p.h>
#include <stdbool.h>
#include <types.h>

#include <avp/clk-rst.h>
#include <avp/io.h>
#include <avp/iomap.h>
#include <avp/pinmux.h>
#include <avp/pmc.h>
#include <avp/timer.h>
#include <avp/uart.h>
#include <avp/usb.h>

void start(void)
{
	struct nv3p nv3p;

	clock_osc_init(&clk_rst);
	pinmux_init(&pinmux);

	uart_init(debug);
	uart_puts(debug, "\n");
	uart_puts(debug, "NVIDIA Tegra Miniloader v" CONFIG_RELEASE "\n");
	uart_puts(debug, "\n");

	usb_init(&usbd);
	usb_enumerate(&usbd);

	nv3p_init(&nv3p, &usbd);
	nv3p_process(&nv3p);

	/* should never get here */
	uart_flush(debug);
	pmc_reset(&pmc, PMC_RESET_MODE_RCM);

	while (1);
}
