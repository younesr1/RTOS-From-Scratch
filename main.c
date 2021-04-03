/*
 * Default main.c for rtos lab.
 * @author Andrew Morton, 2018
 */
#include <LPC17xx.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "rtos.h"

void foo(void *)
{
    while (true)
    {
    }
}

void bar(void *)
{
    while (true)
    {
    }
}

int main(void)
{
    rtos_init();
    tid_t foo_id, bar_id;
    assert(rtos_create_task(foo_id, 5, foo, NULL));
    assert(rtos_create_task(foo_id, 10, bar, NULL));
    rtos_start();
    while (true)
    {
    }
}
