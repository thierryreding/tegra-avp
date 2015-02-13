#ifndef IRQ_H
#define IRQ_H

#define IRQ_NONE    0
#define IRQ_HANDLED 1

typedef int (*irq_handler_t)(unsigned int irq, void *data);

void irq_init(void);
int request_irq(unsigned int irq, irq_handler_t handler, void *data,
		unsigned long flags);
void free_irq(unsigned int irq, void *data);

#define INT_USB 20

#endif
