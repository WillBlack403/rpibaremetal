#ifndef __DECIMAL_TIME_H__
#define __DECIMAL_TIME_H__
#include "mydef.h"

extern const word_t tic_per_metric_second;
extern const word_t metric_seconds_per_day;
extern uint32_t current_metric_time;

void decimal_add_second();
void decimal_add_minute();
void decimal_add_hour();

#endif