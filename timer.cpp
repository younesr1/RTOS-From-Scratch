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
    /* Must disable interrupts since non re-entrant */
    __disable_irq();
    msTicks++;
    if (msTicks % 100 == 0)
    {
        SCB->ICSR = 0x1 << 28;
    }
    __enable_irq();
}