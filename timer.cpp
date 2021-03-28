#include "timer.h"
#include "LPC17xx.h"

volatile uint32_t msTicks = 0;

bool timer_init()
{
    return SysTick_Config(SystemCoreClock / 1000) != 0;
}

uint32_t timer_get_time()
{
    return msTicks;
}

/* SysTick interrupt Handler. */
void SysTick_Handler(void)
{
    msTicks++;
}