#pragma once

#include "pico/time.h"

int scheduler_init();

/**
 * @brief Add tasks to scheduler.
 *
 * All tasks must be added before the scheduler is run. Returns 0 if task was
 * successfully added and 1 if task table is full.
 *
 * @param task Pointer to task function
 * @param period Task run period in microseconds
 * @return int
 */
int scheduler_add_task(void (*task)(), uint64_t period);

/**
 * @brief Run scheduler loop.
 *
 * The loop iterates through every registered task and runs it if its period
 * has elapsed. The elapsed time is recorded before the function is run so
 * the runtime doesn't affect the period.
 *
 * After a task is run, the loop exits and returns the index of that task. Any
 * tasks of a lower priority that are also ready will not be run until the
 * next call of `scheduler_run()`.
 *
 * @return int
 */
int scheduler_run();
