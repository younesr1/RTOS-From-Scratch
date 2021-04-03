#ifndef RTOS_H
#define RTOS_H
#include <stdint.h>

typedef void (*rtos_task_func_t)(void *args);
typedef uint8_t tid_t;

/**
 * @brief Initializes internal variables
 */
void rtos_init(void);

/**
 * @brief Run the scheduler
 */
void rtos_start(void);

/**
 * @brief Create a task
 * @param thread_id Memory address to store thread_id
 * @param priority Task priority
 * @param task Pointer to task
 * @param args args for task function
 * @return true if successful
 */
bool rtos_create_task(tid_t *thread_id, uint8_t priority, rtos_task_func_t task, void *args);

/**
 * @brief Yields to other tasks
 */
//void rtos_task_yield(tid_t id); // this link has the code https://mcuoneclipse.com/2016/08/28/arm-cortex-m-interrupts-and-freertos-part-3/#:~:text=PendSV%20(Pendable%20SerVice)%20is%20an,FreeRTOS%20to%20start%20the%20scheduler.

#endif
