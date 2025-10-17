/**
 * coro_stackless.c
 * Stackless Coroutine Library Implementation
 * 
 * This implementation uses a state machine approach where coroutines
 * save their execution point and resume from there on the next call.
 * No separate stack is maintained, making context switches extremely fast.
 */

#include "coro_stackless.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Global coroutine pool */
static coro_stackless_t coro_pool[MAX_COROUTINES];
static bool initialized = false;
static int current_coro_id = -1;

/* Coroutine function storage */
static coro_func_t coro_functions[MAX_COROUTINES];
static void *coro_args[MAX_COROUTINES];

/**
 * Initialize the coroutine system
 */
void coro_stackless_init(void) {
    if (initialized) return;
    
    memset(coro_pool, 0, sizeof(coro_pool));
    memset(coro_functions, 0, sizeof(coro_functions));
    memset(coro_args, 0, sizeof(coro_args));
    
    for (int i = 0; i < MAX_COROUTINES; i++) {
        coro_pool[i].id = i;
        coro_pool[i].active = false;
        coro_pool[i].state = CORO_STATE_INIT;
        coro_pool[i].resume_point = 0;
        coro_pool[i].user_data = NULL;
    }
    
    initialized = true;
}

/**
 * Find an available coroutine slot
 */
static int find_free_slot(void) {
    for (int i = 0; i < MAX_COROUTINES; i++) {
        if (!coro_pool[i].active) {
            return i;
        }
    }
    return -1;
}

/**
 * Create a new coroutine
 */
int coro_stackless_create(coro_func_t func, void *arg) {
    if (!initialized) {
        coro_stackless_init();
    }
    
    int slot = find_free_slot();
    if (slot == -1) {
        fprintf(stderr, "Error: Maximum coroutines reached\n");
        return -1;
    }
    
    coro_pool[slot].active = true;
    coro_pool[slot].state = CORO_STATE_INIT;
    coro_pool[slot].resume_point = 0;
    coro_pool[slot].user_data = NULL;
    
    coro_functions[slot] = func;
    coro_args[slot] = arg;
    
    return slot;
}

/**
 * Resume execution of a coroutine
 */
int coro_stackless_resume(int coro_id) {
    if (coro_id < 0 || coro_id >= MAX_COROUTINES) {
        return -1;
    }
    
    if (!coro_pool[coro_id].active) {
        return -1;
    }
    
    if (coro_pool[coro_id].state == CORO_STATE_FINISHED) {
        return 1;
    }
    
    /* Save previous coroutine context */
    int prev_coro = current_coro_id;
    current_coro_id = coro_id;
    
    /* Set state to running */
    coro_pool[coro_id].state = CORO_STATE_RUNNING;
    
    /* Execute the coroutine function */
    coro_functions[coro_id](&coro_pool[coro_id], coro_args[coro_id]);
    
    /* Check final state */
    if (coro_pool[coro_id].state == CORO_STATE_RUNNING) {
        coro_pool[coro_id].state = CORO_STATE_SUSPENDED;
    }
    
    /* Restore previous context */
    current_coro_id = prev_coro;
    
    return (coro_pool[coro_id].state == CORO_STATE_FINISHED) ? 1 : 0;
}

/**
 * Yield execution from current coroutine
 */
void coro_stackless_yield(coro_stackless_t *coro) {
    coro->state = CORO_STATE_SUSPENDED;
}

/**
 * Destroy a coroutine
 */
void coro_stackless_destroy(int coro_id) {
    if (coro_id < 0 || coro_id >= MAX_COROUTINES) {
        return;
    }
    
    coro_pool[coro_id].active = false;
    coro_pool[coro_id].state = CORO_STATE_INIT;
    coro_pool[coro_id].resume_point = 0;
    coro_pool[coro_id].user_data = NULL;
    
    coro_functions[coro_id] = NULL;
    coro_args[coro_id] = NULL;
}

/**
 * Cleanup entire coroutine system
 */
void coro_stackless_cleanup(void) {
    for (int i = 0; i < MAX_COROUTINES; i++) {
        if (coro_pool[i].active) {
            coro_stackless_destroy(i);
        }
    }
    initialized = false;
}

/**
 * Get coroutine state
 */
coro_state_t coro_stackless_get_state(int coro_id) {
    if (coro_id < 0 || coro_id >= MAX_COROUTINES) {
        return CORO_STATE_INIT;
    }
    return coro_pool[coro_id].state;
}
