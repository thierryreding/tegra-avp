#ifndef AVP_IO_H
#define AVP_IO_H

static inline __attribute__((always_inline))
unsigned long readl(unsigned long address)
{
	return *(volatile unsigned long *)address;
}

static inline __attribute__((always_inline))
void writel(unsigned long value, unsigned long address)
{
	*(volatile unsigned long *)address = value;
}

#endif
