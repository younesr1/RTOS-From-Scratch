#include "timer.h"
#include "LPC17xx.h"

volatile uint32_t msTicks = 0;

bool timer_init(void)
{
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
    if (msTicks % 100 == 0)
    {
			// younes todo call fpp here. maybe move this function to rtos.c
        SCB->ICSR = 0x1 << 28;
    }
    __enable_irq();
}
