#include "scheduler.h"

#include "config.h"

#include "hardware/gpio.h"
#include "pico/time.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    void (*task)();
    uint64_t period;
    absolute_time_t last_run;
} task_t;

static task_t task_table[SCHEDULER_MAX_TASKS];

int scheduler_init() {
    if (SCHEDULER_MAX_TASKS < 1) {
        return 1;
    }

    for (int i = 0; i < SCHEDULER_MAX_TASKS; i++) {
        // Clear task, don't bother with other fields
        task_table[i].task = NULL;
    }

    return 0;
}

int scheduler_add_task(void (*task)(), uint64_t period) {
    for (int i = 0; i < SCHEDULER_MAX_TASKS; i++) {
        if (task_table[i].task == NULL) {
            task_table[i].task = task;
            task_table[i].period = period;

            // Start counter when task is added
            task_table[i].last_run = get_absolute_time();

            return 0;
        }
    }

    // Table is full
    return 1;
}

int scheduler_run() {
    // TODO: is it best to restart the loop when a task is run?

    for (int i = 0; i < SCHEDULER_MAX_TASKS; i++) {
        // Skip if task doesn't exist
        if (task_table[i].task == NULL) {
            continue;
        }

        uint64_t period = task_table[i].period;

        // Skip if period is 0
        if (period == 0) {
            continue;
        }

        absolute_time_t next = delayed_by_us(task_table[i].last_run, period);
        absolute_time_t now = get_absolute_time();

        // Run if period has elapsed
        if (absolute_time_diff_us(next, now) >= 0) {
            gpio_put(DEBUG_PIN, true);
            task_table[i].task();
            gpio_put(DEBUG_PIN, false);

            // TODO: use next or now here? How do we want to handle things
            // when tasks can't be run fast enough?
            task_table[i].last_run = now;

            // Return the index of the task that was run
            return i;
        }
    }

    // TODO: idle tasks to run when no other tasks are ready?
    // stuff like filesystem maintainence, low priority logs, etc

    // TODO: how useful is it to return the index if we can't easily correlate
    // it to a function?

    return -1;
}
