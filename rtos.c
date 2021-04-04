#include "rtos.h"
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "timer.h"
#include <LPC17xx.h>

#define MAX_THREADS 6
#define MAX_PROCESS_STACK_SIZE 1024
#define MAX_MAIN_STACK_SIZE 2048

#define IDLE_TASK 0

static uint32_t initial_sp = *(uint32_t *)0x0;

static uint8_t active_tasks = 0;

typedef struct TCB
{
	tid_t id;
	uint8_t priority;

	size_t stack_capacity;
	void *stack_base;
	void *stack_top;

} TCB_t;

static TCB_t tcbs[MAX_THREADS];

static TCB_t *active_tcb = NULL;
static TCB_t *next_tcb = NULL;

/* Public Functions */
void rtos_init(void);
void rtos_start(void);
bool rtos_create_task(tid_t *thread_id, uint8_t priority, rtos_task_func_t task, void *args);
void rtos_task_yield(tid_t id);

/* Private Functions */
void rtos_idle_task(void *unused);
__asm void PendSV_Handler(void);
__asm void rtos_initialize_stack(rtos_task_func_t task, void *args);

void rtos_init(void)
{
	NVIC_SetPriority(SysTick_IRQn, 0x00);
	NVIC_SetPriority(PendSV_IRQn, 0xFF);
	for (uint8_t i = 0; i < MAX_THREADS; i++)
	{
		tcbs[i].id = i;
		tcbs[i].priority = 0;
		tcbs[i].stack_capacity = MAX_PROCESS_STACK_SIZE;
		tcbs[i].stack_base = (void *)((initial_sp - MAX_MAIN_STACK_SIZE) - ((MAX_THREADS - i - 1) * MAX_PROCESS_STACK_SIZE));
		tcbs[i].stack_top = tcbs[i].stack_base;
	}
}

void rtos_start(void)
{
	/* At least one task (other than idle) must have been created */
	assert(active_tasks > 1);
	/* Reset MSP to top of main stack frame */
	__set_MSP(initial_sp);
	/* Switch from MSP to PSP and init PSP to idle task stack */
	uint32_t control_register = __get_CONTROL();
	control_register |= 2U;
	__set_CONTROL(control_register);
	__set_PSP((uint32_t)tcbs[IDLE_TASK].stack_base);
	/* Ensure pipe is cleared before loading new instructions since stack change */
	__ISB();
	/* Configure SysTick and call idle task */
	assert(timer_init());
	active_tcb = &tcbs[IDLE_TASK];
	next_tcb = active_tcb;
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
		tcbs[*thread_id].priority = priority;
		rtos_initialize_stack(task, args);
		success = true;
	}
	return success;
}

__asm void rtos_initialize_stack(rtos_task_func_t task, void *args)
{
	PUSH 0x01000000;
	PUSH __cpp(task);
	PUSH LR;
	PUSH R12;
	PUSH R3;
	PUSH R2;
	PUSH R1;
	PUSH __cpp(args);
	PUSH R11;
	PUSH R10;
	PUSH R9;
	PUSH R8;
	PUSH R7;
	PUSH R6;
	PUSH R5;
	PUSH R4;
	BX LR;
}

__asm void PendSV_Handler(void)
{
	// disable interrupts since non re-entrant function
	CPSID i;
	// Save PSP to r0
	MRS R0, PSP;
	// copy r4-r11 to process stack
	STMFD R0 !, {R4 - R11};
	// store top of stack to global var
	LDR R1, __cpp(&active_tcb->stack_top);
	STR R0, [R1];
	// allow pipeline to clear before switching stacks
	ISB;
	// store top of next stack to R2
	LDR R2, __cpp(&next_tcb->stack_top);
	// update the psp to use new stack
	MSR PSP, R2;
	// pop r4-r11 onto the current stack
	LDMFD R2 !, {R4 - R11};
	// return from handler
	BX LR;
	// re-enable interrupts
	CPSIE i;
}

void rtos_idle_task(void *unused)
{
	(void)unused;
	while (true)
	{
		__NOP();
	}
}
