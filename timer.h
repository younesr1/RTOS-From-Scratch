#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 * @brief Configure Systick to run every ms
 */
void timer_init();

/**
 * @brief Get time elapsed in ms
 * @return Elapsed ms
 */
uint32_t timer_get_time();

#endif