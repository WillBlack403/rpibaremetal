#include "TIME/decimaltime.h"
#include "TIME/timer.h"

const word_t tic_per_metric_second = 864000;
const word_t metric_seconds_per_day = 100000;
uint32_t current_metric_time = 0;
uint64_t time_last_suc_button = 0;


int decimal_smooth_out(uint64_t recv_pressed)
{
    return (recv_pressed - time_last_suc_button) > 8333;
}

void decimal_add_second(void)
{
    if(decimal_smooth_out(timer_get_monotonic()))
    {
        current_metric_time += 1;
        time_last_suc_button = timer_get_monotonic();
    }

    //current_metric_time %= metric_seconds_per_day;
}

void decimal_add_minute(void)
{
    if(decimal_smooth_out(timer_get_monotonic()))
    {
        current_metric_time += 100;
        time_last_suc_button = timer_get_monotonic();
    }
}

void decimal_add_hour(void)
{
    if(decimal_smooth_out(timer_get_monotonic()))
    {
        current_metric_time += 10000;
        time_last_suc_button = timer_get_monotonic();
    }
}
