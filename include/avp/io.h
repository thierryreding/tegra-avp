#ifndef AVP_IO_H
#define AVP_IO_H

#include <common.h>
#include <types.h>

#include <avp/uart.h>

static uint32_t __always_inline readl(unsigned long address)
{
	return *(volatile uint32_t *)address;
}

static void __always_inline writel(uint32_t value, unsigned long address)
{
	*(volatile uint32_t *)address = value;
}

#endif
