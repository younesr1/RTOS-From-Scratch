#include "rtos.h"
#include <stddef.h>
#include <stdbool.h>
#include <cassert>
#include "timer.h"
#include <LPC17xx.h>

#define MAX_THREADS 6
#define MAX_PROCESS_STACK_SIZE 1024
#define MAX_MAIN_STACK_SIZE 2048

#define IDLE_TASK 0

extern int __initial_sp; // younes todo questions for morton

static uint8_t active_tasks = 0;

typedef struct TCB
{
	tid_t id;
	uint8_t priority;
	bool active;

	size_t stack_capacity;
	void *stack_base;
	void *stack_top;

} TCB_t;

static TCB_t tcbs[MAX_THREADS];

/* Public Functions */
void rtos_init(void);
void rtos_start(void);
bool rtos_create_task(tid_t *thread_id, uint8_t priority, rtos_task_func_t task, void *args);
void rtos_task_yield(tid_t id);

/* Private Functions */
void rtos_context_switch(void);
void rtos_idle_task(void *);
__asm void PendSV_Handler(void);

void rtos_init(void)
{
	for (uint8_t i = 0; i < MAX_THREADS; i++)
	{
		tcbs[i].id = i;
		tcbs[i].active = false;
		tcbs[i].priority = 0;
		tcbs[i].stack_capacity = MAX_PROCESS_STACK_SIZE;
		tcbs[i].stack_base = (void *)((__initial_sp - MAX_MAIN_STACK_SIZE) - ((MAX_THREADS - i - 1) * MAX_PROCESS_STACK_SIZE));
		tcbs[i].stack_top = tcbs[i].stack_base;
	}
}

void rtos_start(void)
{
	/* At least one task must have been created */
	assert(tcbs[1].active);
	/* Reset MSP to top of main stack frame */
	__set_MSP(__initial_sp);
	/* Switch from MSP to PSP and init PSP to idle task stack */
	uint32_t control_register = __get_CONTROL();
	control_register |= 2U;
	__set_CONTROL(control_register);
	__set_PSP((uint32_t)tcbs[IDLE_TASK].stack_base);
	/* Ensure pipe is cleared before loading new instructions since stack change */
	__ISB();
	/* Configure SysTick and call idle task */
	assert(timer_init());
	tcbs[IDLE_TASK].active = true;
	active_tasks++;
	rtos_idle_task(NULL);
}

bool rtos_create_task(tid_t *thread_id, uint8_t priority, rtos_task_func_t task, void *args)
{
	assert(thread_id != NULL && task != NULL);
	bool success;
	if (active_tasks == (MAX_THREADS - 1))
	{
		success = false;
	}
	else
	{
		active_tasks++;
		*thread_id = active_tasks;
		tcbs[thread_id].priority = priority;
	}
	return success;
}

void rtos_context_switch(void)
{
}

__asm void PendSV_Handler(void)
{
	// return from handler
	BX LR
}

void rtos_idle_task(void *)
{
	// younes todo maybe call rtos yield
	while (true)
		;
}