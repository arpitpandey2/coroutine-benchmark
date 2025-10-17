/**
 * bench.c
 * Coroutine Performance Benchmark Suite
 * 
 * This program benchmarks context-switch performance for both
 * stackless and stackful (ucontext) coroutine implementations.
 * Measures time in nanoseconds using high-resolution clock.
 */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "coro_stackless.h"
#include "coro_ucontext.h"

/* Number of context switches to perform */
#define NUM_SWITCHES 10000000  /* 10 million switches */
#define WARMUP_SWITCHES 100000  /* Warmup iterations */

/* Statistical sampling */
#define NUM_SAMPLES 10

/**
 * Get current time in nanoseconds
 */
static inline long long get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

/* ============================================================
 * STACKLESS COROUTINE BENCHMARKS
 * ============================================================ */

/* Simple ping-pong coroutine for stackless */
static void stackless_worker(coro_stackless_t *coro, void *arg) {
    int *counter = (int *)arg;
    
    CORO_BEGIN(coro);
    
    while (*counter < NUM_SWITCHES) {
        (*counter)++;
        CORO_YIELD(coro);
    }
    
    CORO_END(coro);
}

/**
 * Benchmark stackless coroutine context switches
 */
double benchmark_stackless(void) {
    int counter = 0;
    
    coro_stackless_init();
    
    /* Create two coroutines for ping-pong */
    int coro1 = coro_stackless_create(stackless_worker, &counter);
    int coro2 = coro_stackless_create(stackless_worker, &counter);
    
    if (coro1 < 0 || coro2 < 0) {
        fprintf(stderr, "Failed to create stackless coroutines\n");
        return -1.0;
    }
    
    /* Warmup */
    counter = 0;
    while (counter < WARMUP_SWITCHES) {
        coro_stackless_resume(coro1);
        coro_stackless_resume(coro2);
    }
    
    /* Actual benchmark */
    counter = 0;
    long long start = get_time_ns();
    
    while (counter < NUM_SWITCHES) {
        coro_stackless_resume(coro1);
        coro_stackless_resume(coro2);
    }
    
    long long end = get_time_ns();
    long long total_time = end - start;
    
    /* Calculate average time per switch */
    double avg_ns = (double)total_time / NUM_SWITCHES;
    
    /* Cleanup */
    coro_stackless_destroy(coro1);
    coro_stackless_destroy(coro2);
    coro_stackless_cleanup();
    
    return avg_ns;
}

/* ============================================================
 * UCONTEXT COROUTINE BENCHMARKS
 * ============================================================ */

static int ucontext_counter = 0;

/* Simple worker for ucontext */
static void ucontext_worker(void *arg) {
    while (ucontext_counter < NUM_SWITCHES) {
        ucontext_counter++;
        coro_ucontext_yield();
    }
}

/**
 * Benchmark ucontext coroutine context switches
 */
double benchmark_ucontext(void) {
    ucontext_counter = 0;
    
    coro_ucontext_init();
    
    /* Create two coroutines for ping-pong */
    int coro1 = coro_ucontext_create(ucontext_worker, NULL);
    int coro2 = coro_ucontext_create(ucontext_worker, NULL);
    
    if (coro1 < 0 || coro2 < 0) {
        fprintf(stderr, "Failed to create ucontext coroutines\n");
        return -1.0;
    }
    
    /* Warmup */
    ucontext_counter = 0;
    while (ucontext_counter < WARMUP_SWITCHES) {
        coro_ucontext_resume(coro1);
        coro_ucontext_resume(coro2);
    }
    
    /* Actual benchmark */
    ucontext_counter = 0;
    long long start = get_time_ns();
    
    while (ucontext_counter < NUM_SWITCHES) {
        coro_ucontext_resume(coro1);
        coro_ucontext_resume(coro2);
    }
    
    long long end = get_time_ns();
    long long total_time = end - start;
    
    /* Calculate average time per switch */
    double avg_ns = (double)total_time / NUM_SWITCHES;
    
    /* Cleanup */
    coro_ucontext_destroy(coro1);
    coro_ucontext_destroy(coro2);
    coro_ucontext_cleanup();
    
    return avg_ns;
}

/**
 * Calculate statistics from samples
 */
void calculate_stats(double *samples, int n, double *mean, double *min, double *max) {
    *mean = 0.0;
    *min = samples[0];
    *max = samples[0];
    
    for (int i = 0; i < n; i++) {
        *mean += samples[i];
        if (samples[i] < *min) *min = samples[i];
        if (samples[i] > *max) *max = samples[i];
    }
    
    *mean /= n;
}

/**
 * Main benchmark driver
 */
int main(int argc, char *argv[]) {
    printf("=======================================================\n");
    printf("  Coroutine Context-Switch Benchmark Suite\n");
    printf("=======================================================\n");
    printf("Number of switches: %d\n", NUM_SWITCHES);
    printf("Number of samples: %d\n", NUM_SAMPLES);
    printf("-------------------------------------------------------\n\n");
    
    /* Determine which benchmark to run */
    char *benchmark_type = (argc > 1) ? argv[1] : "both";
    
    if (strcmp(benchmark_type, "stackless") == 0 || strcmp(benchmark_type, "both") == 0) {
        printf("Running STACKLESS coroutine benchmark...\n");
        fflush(stdout);
        
        double stackless_samples[NUM_SAMPLES];
        for (int i = 0; i < NUM_SAMPLES; i++) {
            stackless_samples[i] = benchmark_stackless();
            printf("  Sample %d: %.2f ns/switch\n", i + 1, stackless_samples[i]);
            fflush(stdout);
        }
        
        double mean, min, max;
        calculate_stats(stackless_samples, NUM_SAMPLES, &mean, &min, &max);
        
        printf("\nStackless Results:\n");
        printf("  Mean:  %.2f ns/switch\n", mean);
        printf("  Min:   %.2f ns/switch\n", min);
        printf("  Max:   %.2f ns/switch\n", max);
        printf("-------------------------------------------------------\n\n");
        
        /* Save results to file */
        FILE *f = fopen("stackless_results.txt", "w");
        if (f) {
            fprintf(f, "mean=%.2f\n", mean);
            fprintf(f, "min=%.2f\n", min);
            fprintf(f, "max=%.2f\n", max);
            fclose(f);
            printf("Results saved to stackless_results.txt\n\n");
        }
    }
    
    if (strcmp(benchmark_type, "ucontext") == 0 || strcmp(benchmark_type, "both") == 0) {
        printf("Running UCONTEXT coroutine benchmark...\n");
        fflush(stdout);
        
        double ucontext_samples[NUM_SAMPLES];
        for (int i = 0; i < NUM_SAMPLES; i++) {
            ucontext_samples[i] = benchmark_ucontext();
            printf("  Sample %d: %.2f ns/switch\n", i + 1, ucontext_samples[i]);
            fflush(stdout);
        }
        
        double mean, min, max;
        calculate_stats(ucontext_samples, NUM_SAMPLES, &mean, &min, &max);
        
        printf("\nUcontext Results:\n");
        printf("  Mean:  %.2f ns/switch\n", mean);
        printf("  Min:   %.2f ns/switch\n", min);
        printf("  Max:   %.2f ns/switch\n", max);
        printf("-------------------------------------------------------\n\n");
        
        /* Save results to file */
        FILE *f = fopen("ucontext_results.txt", "w");
        if (f) {
            fprintf(f, "mean=%.2f\n", mean);
            fprintf(f, "min=%.2f\n", min);
            fprintf(f, "max=%.2f\n", max);
            fclose(f);
            printf("Results saved to ucontext_results.txt\n\n");
        }
    }
    
    printf("=======================================================\n");
    printf("Benchmark completed successfully!\n");
    printf("=======================================================\n");
    
    return 0;
}
