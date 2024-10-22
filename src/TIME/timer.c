#include "TIME/timer.h"
#include "TIME/decimaltime.h"
#include "GPIO/gpio.h"
#include "mydef.h"
#include "interrupt.h"
volatile __attribute__((aligned(4))) timer_registers * const timer = (__attribute__((aligned(4)))timer_registers * const)0x20003000;

uint64_t 
timer_get_monotonic(void)
{
    uint64_t current_tic = 0;
    current_tic = ((uint64_t) timer->higher << 32) | (uint64_t) timer->lower;
    return current_tic;
}

void
timer_spin_wait(uint64_t us)
{
    us += timer_get_monotonic();
    while(timer_get_monotonic() < us) {};
}

void timer_update(word_t cycles)
{
    dmp();
    timer->control_status = 0x1 << IRQ_TIMER;
    timer->cmp[IRQ_TIMER] = timer -> lower + cycles;
    dmp();
}

extern void update(void);
interrupt timer_handler(void)
{
    timer_update(tic_per_metric_second);
    decimal_add_second();
    update();
}