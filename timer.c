#include "timer.h"
#include <stdlib.h>
#include "LPC17xx.h"

volatile uint32_t msTicks = 0;

timer_callback_t tick_cb = NULL;

bool timer_init(timer_callback_t cb)
{
    tick_cb = cb;
    return SysTick_Config(SystemCoreClock / 1000) != 0;
}

uint32_t timer_get_time(void)
{
    return msTicks;
}

/* SysTick interrupt Handler. */
void SysTick_Handler(void)
{
    /* Must disable interrupts since non re-entrant */
    __disable_irq();
    msTicks++;
    if (tick_cb)
    {
        tick_cb(msTicks);
    }
    __enable_irq();
}
