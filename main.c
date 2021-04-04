/*
 * Default main.c for rtos lab.
 * @author Andrew Morton, 2018
 */
#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "rtos.h"

void foo(void *unused)
{
	(void)unused;
    while (true)
    {
    }
}

void bar(void *unused)
{
	(void)unused;
    while (true)
    {
    }
}

int main(void)
{
    rtos_init();
    tid_t foo_id, bar_id;
    assert(rtos_create_task(&foo_id, 5, foo, NULL));
    assert(rtos_create_task(&bar_id, 10, bar, NULL));
    rtos_start();
    while (true)
    {
    }
}
