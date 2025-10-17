/**
 * coro_ucontext.c
 * Stackful Coroutine Library Implementation using ucontext
 * 
 * This implementation uses POSIX ucontext to create coroutines with
 * separate stacks. Context switches involve saving/restoring CPU registers
 * and switching stack pointers, which is slower than stackless approach.
 */

#include "coro_ucontext.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Global coroutine pool */
static coro_ucontext_t ucoro_pool[MAX_UCONTEXT_COROUTINES];
static bool initialized = false;
static int current_ucoro_id = -1;
static ucontext_t main_context;

/* Coroutine function storage */
typedef struct {
    ucoro_func_t func;
    void *arg;
    int coro_id;
} coro_wrapper_args_t;

static coro_wrapper_args_t wrapper_args[MAX_UCONTEXT_COROUTINES];

/**
 * Wrapper function that runs the user's coroutine function
 */
static void coro_wrapper(void) {
    int id = current_ucoro_id;
    
    if (id >= 0 && id < MAX_UCONTEXT_COROUTINES) {
        ucoro_pool[id].state = UCORO_STATE_RUNNING;
        
        /* Execute user function */
        wrapper_args[id].func(wrapper_args[id].arg);
        
        /* Mark as finished */
        ucoro_pool[id].state = UCORO_STATE_FINISHED;
    }
    
    /* Return to caller */
    if (ucoro_pool[id].caller) {
        setcontext(ucoro_pool[id].caller);
    }
}

/**
 * Initialize the ucontext coroutine system
 */
void coro_ucontext_init(void) {
    if (initialized) return;
    
    memset(ucoro_pool, 0, sizeof(ucoro_pool));
    memset(wrapper_args, 0, sizeof(wrapper_args));
    
    for (int i = 0; i < MAX_UCONTEXT_COROUTINES; i++) {
        ucoro_pool[i].id = i;
        ucoro_pool[i].active = false;
        ucoro_pool[i].state = UCORO_STATE_INIT;
        ucoro_pool[i].stack = NULL;
        ucoro_pool[i].caller = NULL;
    }
    
    initialized = true;
}

/**
 * Find an available coroutine slot
 */
static int find_free_ucoro_slot(void) {
    for (int i = 0; i < MAX_UCONTEXT_COROUTINES; i++) {
        if (!ucoro_pool[i].active) {
            return i;
        }
    }
    return -1;
}

/**
 * Create a new stackful coroutine
 */
int coro_ucontext_create(ucoro_func_t func, void *arg) {
    if (!initialized) {
        coro_ucontext_init();
    }
    
    int slot = find_free_ucoro_slot();
    if (slot == -1) {
        fprintf(stderr, "Error: Maximum ucontext coroutines reached\n");
        return -1;
    }
    
    /* Allocate stack */
    char *stack = (char *)malloc(CORO_STACK_SIZE);
    if (!stack) {
        fprintf(stderr, "Error: Failed to allocate coroutine stack\n");
        return -1;
    }
    
    /* Initialize context */
    if (getcontext(&ucoro_pool[slot].context) == -1) {
        free(stack);
        return -1;
    }
    
    ucoro_pool[slot].context.uc_stack.ss_sp = stack;
    ucoro_pool[slot].context.uc_stack.ss_size = CORO_STACK_SIZE;
    ucoro_pool[slot].context.uc_link = NULL;
    
    /* Store function and arguments */
    wrapper_args[slot].func = func;
    wrapper_args[slot].arg = arg;
    wrapper_args[slot].coro_id = slot;
    
    /* Create context */
    makecontext(&ucoro_pool[slot].context, coro_wrapper, 0);
    
    /* Set coroutine as active */
    ucoro_pool[slot].active = true;
    ucoro_pool[slot].state = UCORO_STATE_INIT;
    ucoro_pool[slot].stack = stack;
    ucoro_pool[slot].caller = &main_context;
    
    return slot;
}

/**
 * Resume execution of a coroutine
 */
int coro_ucontext_resume(int coro_id) {
    if (coro_id < 0 || coro_id >= MAX_UCONTEXT_COROUTINES) {
        return -1;
    }
    
    if (!ucoro_pool[coro_id].active) {
        return -1;
    }
    
    if (ucoro_pool[coro_id].state == UCORO_STATE_FINISHED) {
        return 1;
    }
    
    /* Save current coroutine ID */
    int prev_id = current_ucoro_id;
    current_ucoro_id = coro_id;
    
    /* Switch to coroutine context */
    ucoro_pool[coro_id].state = UCORO_STATE_RUNNING;
    swapcontext(&main_context, &ucoro_pool[coro_id].context);
    
    /* Returned from coroutine */
    current_ucoro_id = prev_id;
    
    return (ucoro_pool[coro_id].state == UCORO_STATE_FINISHED) ? 1 : 0;
}

/**
 * Yield execution back to caller
 */
void coro_ucontext_yield(void) {
    if (current_ucoro_id >= 0 && current_ucoro_id < MAX_UCONTEXT_COROUTINES) {
        ucoro_pool[current_ucoro_id].state = UCORO_STATE_SUSPENDED;
        swapcontext(&ucoro_pool[current_ucoro_id].context, &main_context);
    }
}

/**
 * Destroy a coroutine
 */
void coro_ucontext_destroy(int coro_id) {
    if (coro_id < 0 || coro_id >= MAX_UCONTEXT_COROUTINES) {
        return;
    }
    
    if (ucoro_pool[coro_id].stack) {
        free(ucoro_pool[coro_id].stack);
        ucoro_pool[coro_id].stack = NULL;
    }
    
    ucoro_pool[coro_id].active = false;
    ucoro_pool[coro_id].state = UCORO_STATE_INIT;
    ucoro_pool[coro_id].caller = NULL;
}

/**
 * Cleanup entire coroutine system
 */
void coro_ucontext_cleanup(void) {
    for (int i = 0; i < MAX_UCONTEXT_COROUTINES; i++) {
        if (ucoro_pool[i].active) {
            coro_ucontext_destroy(i);
        }
    }
    initialized = false;
}

/**
 * Get coroutine state
 */
ucoro_state_t coro_ucontext_get_state(int coro_id) {
    if (coro_id < 0 || coro_id >= MAX_UCONTEXT_COROUTINES) {
        return UCORO_STATE_INIT;
    }
    return ucoro_pool[coro_id].state;
}
