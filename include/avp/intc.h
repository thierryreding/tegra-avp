#ifndef AVP_INTC_H
#define AVP_INTC_H

struct intc {
	unsigned long base;
};

void intc_enable_irq(struct intc *intc, unsigned int irq);
void intc_disable_irq(struct intc *intc, unsigned int irq);
void intc_ack_irq(struct intc *intc, unsigned int irq);

extern struct intc intc;

#endif
