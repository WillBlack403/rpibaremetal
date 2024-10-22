#ifndef __MYDEF_H__
#define __MYDEF_H__
#include"myint.h"

typedef uint32_t word_t;
typedef void interrupt;
typedef interrupt (*interrupt_handler_t)(void);
typedef void callback;
typedef callback (*callback_t)(void);
#define NULL ((void *)0)
typedef word_t size_t;



#endif