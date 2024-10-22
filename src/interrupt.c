#include "interrupt.h"

volatile interrupt_registers * const IRQ = (volatile interrupt_registers * const)0x2000B200;
interrupt_handler_t handlers[96];
uint32_t enabled_irq[3];


void enable_irq(uint8_t irq_num,interrupt_handler_t handle)
{
    if(irq_num < 32)
    {
        IRQ-> enable_IRQ[0] = 0x1 << irq_num;
        enabled_irq[0] |= 0x1 << irq_num;
    }
    else if(irq_num < 64)
    {
        IRQ -> enable_IRQ[1] = (0x1 << (irq_num - 32));
        enabled_irq[1] |= (0x1 << (irq_num - 32));
    }
    else
    {
        IRQ -> enable_basic_IRQ = (0x1 << (irq_num - 64));
        enabled_irq[2] |= (0x1 << (irq_num - 64));
    }
    handlers[irq_num] = handle;
}


void pending(uint8_t irq_num)
{
    //if(IRQ_GPIO == irq_num) toggle();
    if(irq_num < 64 &&  IRQ -> IRQ_pending[irq_num/32] & (1 << (irq_num - (irq_num/32)*32)))
        (handlers[irq_num])();
    else if( IRQ -> IRQ_basic_pending & (1 << (irq_num - 64)))
        (handlers[irq_num])();

    /*if(irq_num < 32)
    {
        IRQ-> enable_IRQ[0] = 0x1 << irq_num;
        enabled_irq[0] |= 0x1 << irq_num;
    }
    else if(irq_num < 64)
    {
        IRQ -> enable_IRQ[1] = (0x1 << (irq_num - 32));
        enabled_irq[1] |= (0x1 << (irq_num - 32));
    }
    else
    {
        IRQ -> enable_basic_IRQ = (0x1 << (irq_num - 64));
        enabled_irq[2] |= (0x1 << (irq_num - 64));
    }*/
}


void disbatch(void)
{
    for(uint8_t i = 0; i < 3; i++)
    {
        word_t mask = enabled_irq[i];
        while(mask != 0)
        {
            word_t bit = 31 - __builtin_clz(mask);
            mask ^= (1 << bit);
            pending(bit + (i<<5));
        }
    }
}