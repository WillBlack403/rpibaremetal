#ifndef __GPIO_GPIO_H__
#define __GPIO_GPIO_H__

#include"mydef.h"

typedef enum {
    FSEL_IN   = 0x0, 
    FSEL_OUT  = 0x1, 
    FSEL_ALT5 = 0x2, 
    FSEL_ALT4 = 0x3, 
    FSEL_ALT0 = 0x4, 
    FSEL_ALT1 = 0x5, 
    FSEL_ALT2 = 0x6,
    FSEL_ALT3 = 0x7,
} GPIOMODE;

typedef union
{
    struct{
        
        GPIOMODE pin0 : 3;
        GPIOMODE pin1 : 3;
        GPIOMODE pin2 : 3;
        GPIOMODE pin3 : 3;
        GPIOMODE pin4 : 3;
        GPIOMODE pin5 : 3;
        GPIOMODE pin6 : 3;
        GPIOMODE pin7 : 3;
        GPIOMODE pin8 : 3;
        GPIOMODE pin9 : 3;
        unsigned _unused : 2;
    };
    word_t raw;
} gpio_fsel;

struct __attribute__((__packed__,aligned(4))) gpio_registers
{
    word_t      fsel[6];
    word_t         _res0;
    word_t         set[2];
    word_t         _res1;
    word_t         clear[2];
    word_t         _res3;
    const word_t   level[2];
    word_t         _res4;
    word_t         event[2];
    word_t         _res5;
    word_t         rising_detect[2];
    word_t         _res6;
    word_t         falling_detect[2];
    word_t         _res7;
    word_t         high[2];
    word_t         _res8;
    word_t         low[2];
    word_t         _res9;
    word_t         async_rising_detect[2];
    word_t         _res10;
    word_t         async_falling_detect[2];
    word_t         _res11;
    word_t         pull;
    word_t         pull_clock[2];
    word_t         _res12;
    word_t         test;
};



void   gpio_function(uint8_t, GPIOMODE);
//void   gpio_set(uint8_t);
//void   gpio_clear(uint8_t);
void   gpio_output(uint8_t, int8_t);
void   gpio_enable_high(uint8_t,callback_t);
void   gpio_enable_falling(uint8_t,callback_t);
void   gpio_enable_rising(uint8_t,callback_t);
interrupt gpio_handler(void);


#endif