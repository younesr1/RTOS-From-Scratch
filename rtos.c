#include "rtos.h"
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "timer.h"
#include <LPC17xx.h>

#define MAX_THREADS 6
#define MAX_PROCESS_STACK_SIZE 1024
#define MAX_MAIN_STACK_SIZE 2048

#define PSR_DEFAULT 0x01000000
#define PENDSV_TRIGGER_BIT 28
#define SCHEDULER_TICK_PERIOD 100

#define IDLE_TASK 0

static uint8_t active_tasks = 0;

typedef struct TCB
{
	tid_t id;
	size_t stack_capacity;
	void *stack_base;
	void *stack_top;

} TCB_t;

static TCB_t tcbs[MAX_THREADS];

static uint8_t current_tcb = 0, next_tcb = 0;

/* Public Functions */
void rtos_init(void);
void rtos_start(void);
bool rtos_create_task(tid_t *thread_id, rtos_task_func_t task, void *args);
void rtos_task_yield(tid_t id);

/* Private Functions */
void rtos_idle_task(void *unused);
void rtos_ticker_callback(uint32_t tick);
void rtos_run_scheduler(void); //  Pre-emptive round robin
void rtos_trigger_pendsv(void);
__asm void PendSV_Handler(void);
__asm void rtos_initialize_stack(rtos_task_func_t task, void *args);

void rtos_init(void)
{
	const uint32_t initial_sp = *(uint32_t *)SCB->VTOR;
	NVIC_SetPriority(SysTick_IRQn, 0x00);
	NVIC_SetPriority(PendSV_IRQn, 0xFF);
	for (uint8_t i = 0; i < MAX_THREADS; i++)
	{
		tcbs[i].id = i;
		tcbs[i].stack_capacity = MAX_PROCESS_STACK_SIZE;
		tcbs[i].stack_base = (void *)((initial_sp - MAX_MAIN_STACK_SIZE) - ((MAX_THREADS - i - 1) * MAX_PROCESS_STACK_SIZE));
		tcbs[i].stack_top = tcbs[i].stack_base;
	}
}

void rtos_start(void)
{
	const uint32_t initial_sp = *(uint32_t *)SCB->VTOR;
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
	assert(timer_init(rtos_ticker_callback));
	current_tcb = IDLE_TASK;
	next_tcb = current_tcb;
	active_tasks++;
	rtos_idle_task(NULL);
}

bool rtos_create_task(tid_t *thread_id, rtos_task_func_t task, void *args)
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
		rtos_initialize_stack(task, args);
		success = true;
	}
	return success;
}

__asm void rtos_initialize_stack(const rtos_task_func_t task, void *args)
{
	PUSH PSR_DEFAULT;
	LDR PC, = __cpp(task);
	PUSH PC;
	PUSH LR;
	PUSH R12;
	PUSH R3;
	PUSH R2;
	PUSH R1;
	LDR R0, = __cpp(args);
	PUSH R0;
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
	LDR R1, __cpp(tcbs[current_tcb].stack_top);
	STR R0, [R1];
	// allow pipeline to clear before switching stacks
	ISB;
	// store top of next stack to R2
	LDR R2, __cpp(tcbs[next_tcb].stack_top);
	// update the psp to use new stack
	MSR PSP, R2;
	// pop r4-r11 onto the current stack
	LDMFD R2 !, {R4 - R11};
	// re-enable interrupts
	CPSIE i;
	// return from handler
	BX LR;
}

void rtos_idle_task(void *unused)
{
	(void)unused;
	while (true)
	{
		__NOP();
	}
}

void rtos_ticker_callback(uint32_t tick)
{
	if (tick % SCHEDULER_TICK_PERIOD == 0)
	{
		rtos_run_scheduler();
		rtos_trigger_pendsv();
	}
}

void rtos_trigger_pendsv(void)
{
	SCB->ICSR = 0x1 >> PENDSV_TRIGGER_BIT;
}

void rtos_run_scheduler(void)
{
	next_tcb = (next_tcb + 1) % active_tasks;
}
