/*
 * coro_stackless.h
 * Stackless Coroutine Library Header
 * 
 * This library implements user-space coroutines without maintaining separate stacks.
 * Instead, it uses a state machine approach where each coroutine remembers its 
 * execution state and resumes from that point.
 */

#ifndef CORO_STACKLESS_H
#define CORO_STACKLESS_H

#include <stddef.h>
#include <stdbool.h>

/* Maximum number of coroutines that can be managed */
#define MAX_COROUTINES 1024

/* Coroutine states */
typedef enum {
    CORO_STATE_INIT = 0,      /* Initial state */
    CORO_STATE_RUNNING,       /* Currently executing */
    CORO_STATE_SUSPENDED,     /* Suspended, waiting to resume */
    CORO_STATE_FINISHED       /* Completed execution */
} coro_state_t;

/* Coroutine structure - holds state and user data */
typedef struct {
    int id;                   /* Unique coroutine identifier */
    coro_state_t state;       /* Current execution state */
    int resume_point;         /* State machine resume point */
    void *user_data;          /* User-defined data pointer */
    bool active;              /* Whether this slot is in use */
} coro_stackless_t;

/* Coroutine function pointer type */
typedef void (*coro_func_t)(coro_stackless_t *coro, void *arg);

/**
 * Initialize the stackless coroutine system
 * Must be called before using any other functions
 */
void coro_stackless_init(void);

/**
 * Create a new coroutine
 * Returns: coroutine ID on success, -1 on failure
 */
int coro_stackless_create(coro_func_t func, void *arg);

/**
 * Resume execution of a coroutine
 * Returns: 0 if coroutine yielded, 1 if finished, -1 on error
 */
int coro_stackless_resume(int coro_id);

/**
 * Yield execution from current coroutine
 * Must be called from within a coroutine function
 */
void coro_stackless_yield(coro_stackless_t *coro);

/**
 * Destroy a coroutine and free resources
 */
void coro_stackless_destroy(int coro_id);

/**
 * Cleanup the entire coroutine system
 */
void coro_stackless_cleanup(void);

/**
 * Get coroutine state
 */
coro_state_t coro_stackless_get_state(int coro_id);

/* Macros for implementing state machine logic in coroutines */
#define CORO_BEGIN(coro) switch((coro)->resume_point) { case 0:
#define CORO_YIELD(coro) do { (coro)->resume_point = __LINE__; return; case __LINE__:; } while(0)
#define CORO_END(coro) } (coro)->state = CORO_STATE_FINISHED

#endif /* CORO_STACKLESS_H */
