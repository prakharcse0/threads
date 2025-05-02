#ifndef MY_BARRIER_H
#define MY_BARRIER_H

#include <pthread.h>

/**
 * Barrier synchronization primitive
 * 
 * A barrier is a synchronization point where multiple threads wait until
 * a specified number of threads have reached the barrier, then all proceed.
 */
typedef struct {
    pthread_mutex_t mutex;     // Mutex to protect barrier state
    pthread_cond_t cond;       // Condition variable for waiting threads
    int count;                 // Current number of threads at the barrier
    int trip_count;            // Number of threads needed to release the barrier
    int cycle;                 // Current cycle (to prevent spurious wakeups)
} my_barrier_t;

/**
 * Initialize a barrier
 * 
 * @param barrier Pointer to barrier structure
 * @param count Number of threads that must reach the barrier before any proceed
 * @return 0 on success, error code on failure
 */
int my_barrier_init(my_barrier_t *barrier, int count);

/**
 * Wait on a barrier
 * 
 * Blocks the calling thread until the required number of threads have called wait
 * 
 * @param barrier Pointer to barrier structure
 * @return 0 on success, error code on failure
 */
int my_barrier_wait(my_barrier_t *barrier);

/**
 * Destroy a barrier
 * 
 * @param barrier Pointer to barrier structure
 * @return 0 on success, error code on failure
 */
int my_barrier_destroy(my_barrier_t *barrier);

#endif /* MY_BARRIER_H */
