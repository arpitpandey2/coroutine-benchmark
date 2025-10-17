/**
 * coro_ucontext.h
 * Stackful Coroutine Library Header using POSIX ucontext
 * 
 * This library implements user-space coroutines using the POSIX ucontext API.
 * Each coroutine has its own stack, allowing for more flexible control flow
 * but with higher memory usage and slower context switches.
 */

#ifndef CORO_UCONTEXT_H
#define CORO_UCONTEXT_H

#include <ucontext.h>
#include <stdbool.h>

/* Stack size for each coroutine (64KB) */
#define CORO_STACK_SIZE (64 * 1024)

/* Maximum number of coroutines */
#define MAX_UCONTEXT_COROUTINES 1024

/* Coroutine states */
typedef enum {
    UCORO_STATE_INIT = 0,
    UCORO_STATE_RUNNING,
    UCORO_STATE_SUSPENDED,
    UCORO_STATE_FINISHED
} ucoro_state_t;

/* Stackful coroutine structure */
typedef struct {
    int id;                   /* Unique identifier */
    ucontext_t context;       /* Execution context */
    ucontext_t *caller;       /* Caller's context */
    char *stack;              /* Stack memory */
    ucoro_state_t state;      /* Current state */
    bool active;              /* In use flag */
    void *user_data;          /* User data pointer */
} coro_ucontext_t;

/* Coroutine function pointer type */
typedef void (*ucoro_func_t)(void *arg);

/**
 * Initialize the ucontext coroutine system
 */
void coro_ucontext_init(void);

/**
 * Create a new stackful coroutine
 * Returns: coroutine ID on success, -1 on failure
 */
int coro_ucontext_create(ucoro_func_t func, void *arg);

/**
 * Resume execution of a coroutine
 * Returns: 0 if yielded, 1 if finished, -1 on error
 */
int coro_ucontext_resume(int coro_id);

/**
 * Yield execution from current coroutine back to caller
 */
void coro_ucontext_yield(void);

/**
 * Destroy a coroutine and free resources
 */
void coro_ucontext_destroy(int coro_id);

/**
 * Cleanup entire coroutine system
 */
void coro_ucontext_cleanup(void);

/**
 * Get coroutine state
 */
ucoro_state_t coro_ucontext_get_state(int coro_id);

#endif /* CORO_UCONTEXT_H */
