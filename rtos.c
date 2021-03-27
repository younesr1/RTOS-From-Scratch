#include "rtos.h"
#include <stddef.h>
#include <stdbool.h>
#include <LPC17xx.h>

#define MAX_THREADS 6
#define MAX_PROCESS_STACK_SIZE 1024
#define MAX_MAIN_STACK_SIZE 2048

#define IDLE_TASK 0

typedef uint16_t tid_t;

typedef struct TCB {
	tid_t thread_id; // younes todo might not need this
  bool active;
	void *stack_base;
	size_t stack_capacity;
	
} TCB_t;

static TCB_t tcbs[MAX_THREADS];

void rtos_initialize(void) {
	for(uint8_t i = 0; i < MAX_THREADS; i++) {
		TCB_t *cur = &tcbs[i];
		cur->thread_id = i;
		extern int __initial_sp; // younes todo questions for morton
		cur->stack_base = (void *)((__initial_sp - 2048) - ((MAX_THREADS-i-1) * MAX_PROCESS_STACK_SIZE));
		cur->stack_capacity = MAX_PROCESS_STACK_SIZE;
		cur->active = false;
	}
}

void rtos_start(void) {
	// younes todo assert that a task was created
	// Use PSP over MSP
	uint32_t control_register = __get_CONTROL();
	control_register |= 2U;
	__set_CONTROL(control_register);
}
