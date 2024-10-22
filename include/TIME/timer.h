#ifndef __TIMER_H__
#define __TIMER_H__
#include"mydef.h"
#include"interrupt.h"

typedef struct __attribute__((__packed__,aligned(4)))
{
    word_t  control_status;
    word_t  lower;
    word_t  higher;
    word_t  cmp[4];
}timer_registers;

uint64_t timer_get_monotonic(void);
void timer_spin_wait(uint64_t);
void timer_update(word_t);
interrupt timer_handler(void);



#endif