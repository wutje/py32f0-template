#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "py32f0xx_hal_conf.h"
void raddr_output_init(void);
void raddr_output_print(void);

#define TIMER_DESIRED_BASE_TICK     (10e-6) //x * 1uS
#define TIMER_DIVIDER               ((uint32_t)(HSI_VALUE * TIMER_DESIRED_BASE_TICK))
_Static_assert(TIMER_DIVIDER > 100, "We need at least ~100 CPU cycles for the ISR");
_Static_assert(TIMER_DIVIDER < 65535, "Divider too large. Consider lowering input clock");

#define TIMER_ACTUAL_TIME_PER_TICK (1.0 * TIMER_DIVIDER / HSI_VALUE)
static inline uint16_t us_to_timer_tick(uint16_t tmo) {
    return tmo * (10e-6 / TIMER_ACTUAL_TIME_PER_TICK);
}

void fifo_write(bool bit, uint16_t tmo);

