#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*timer_callback_t)(uint32_t);

/**
 * @brief Configure Systick to run every ms
 */
bool timer_init(timer_callback_t cb);

/**
 * @brief Get time elapsed in ms
 * @return Elapsed ms
 */
uint32_t timer_get_time(void);

#endif
