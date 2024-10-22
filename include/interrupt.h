#ifndef __interrupt_h__
#define __interrupt_h__
#include"mydef.h"


#define IRQ_TIMER 3
#define IRQ_GPIO0 49
#define IRQ_GPIO1 50
#define IRQ_GPIO2 51
#define IRQ_GPIO3 52

typedef struct __attribute__((__packed__,aligned(4)))
{
    word_t IRQ_basic_pending;
    word_t IRQ_pending[2];
    word_t FIQ_control;
    word_t enable_IRQ[2];
    word_t enable_basic_IRQ;
    word_t disable_IRQ[2];
    word_t disable_basic_IRQ;
}interrupt_registers;


void enable(void);
void disable(void);
void enable_irq(uint8_t,interrupt_handler_t);
void dmp(void);
void disbatch(void);


#endif  