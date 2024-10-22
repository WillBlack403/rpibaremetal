//#include"hardware/rawmem.h"
#include "GPIO/gpio.h"
#include "interrupt.h"
#include "TIME/timer.h"




volatile __attribute__((aligned(4))) struct gpio_registers* const gpio = (__attribute__((aligned(4)))struct gpio_registers* const) 0x20200000;
callback_t  register_callbacks[52];
uint32_t    input_registered_pins[2] = {0};


void
gpio_function(uint8_t pin, GPIOMODE mode)
{
    if(pin > 54) return;
    if(mode < 0 || mode > 7) return;
    dmp();
    word_t _curr = gpio->fsel[(pin/10)];
    _curr &= ~(0x7 << pin%10*3);
    _curr |= (mode << pin%10*3);
    gpio->fsel[(pin/10)] = _curr;
    dmp();
}

void
gpio_enable_high(uint8_t pin, callback_t function)
{
    if(pin > 54) return;
    
    if( ((gpio->fsel[pin/10] >> (pin%10*3)) & 0x7 )!= FSEL_IN) return;
    
    dmp();
    gpio->high[pin/32] |= 0x1 << (pin - (pin/32)*32);
    dmp();
    input_registered_pins[pin/32] |= 0x1 << (pin - (pin/32)*32);
    register_callbacks[pin] = function;
}

void
gpio_enable_falling(uint8_t pin, callback_t function)
{
    if(pin > 54) return;
    
    if( ((gpio->fsel[pin/10] >> (pin%10*3)) & 0x7 )!= FSEL_IN) return;
    dmp();
    gpio ->falling_detect[pin/32] |=  0x1 << (pin - (pin/32)*32);
    dmp();
    input_registered_pins[pin/32] |=  0x1 << (pin - (pin/32)*32);
    register_callbacks[pin] = function;
}

void
gpio_enable_rising(uint8_t pin, callback_t function)
{
    if(pin > 54) return;
    
    if( ((gpio->fsel[pin/10] >> (pin%10*3)) & 0x7 )!= FSEL_IN) return;
    dmp();
    gpio ->rising_detect[pin/32] |=  0x1 << (pin - (pin/32)*32);
    dmp();
    input_registered_pins[pin/32] |=  0x1 << (pin - (pin/32)*32);
    register_callbacks[pin] = function;
}

void
gpio_set(uint8_t pin)
{
    dmp();
    gpio->set[pin/32] = 0x1 << (pin - (pin/32)*32);
    dmp();
}

void
gpio_clear(uint8_t pin)
{
    dmp();
    gpio->clear[pin/32] = 0x1 << (pin - (pin/32)*32);
    dmp();
}

void
gpio_output(uint8_t pin, int8_t high)
{
    if(pin > 54) return;
    
    if( ((gpio->fsel[pin/10] >> (pin%10*3)) & 0x7 )!= FSEL_OUT) return;
    (high) ? gpio_set(pin) : gpio_clear(pin);
}

extern void toggle(void);
interrupt gpio_handler(void)
{
    //toggle();
    for(int i = 0; i < 2; i++)
    {
        uint32_t mask = input_registered_pins[i];
        while(mask != 0)
        {
            uint32_t bit = 31 - __builtin_clz(mask);
            mask ^= (1 << bit);
            dmp();
            if((gpio->event[i] & (0x1 << bit)) != 0)
            {
                register_callbacks[bit + (i<<5)]();
                gpio->event[i] |= (0x1 << bit);
            }
            dmp();
        }
    }

}



