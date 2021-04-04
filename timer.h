#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configure Systick to run every ms
 */
bool timer_init();

/**
 * @brief Get time elapsed in ms
 * @return Elapsed ms
 */
uint32_t timer_get_time();

#endif