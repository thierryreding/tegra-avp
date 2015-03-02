#ifndef AVP_TIMER_H
#define AVP_TIMER_H

enum osc_freq;

void timer_us_init(enum osc_freq osc);
void udelay(unsigned long delay);

#endif
