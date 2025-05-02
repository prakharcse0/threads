#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "my_barrier.h"

/**
 * Initialize a barrier
 * 
 * @param barrier Pointer to barrier structure
 * @param count Number of threads that must reach the barrier before any proceed
 * @return 0 on success, error code on failure
 */
int my_barrier_init(my_barrier_t *barrier, int count) {
    if (barrier == NULL || count <= 0) {
        return EINVAL;
    }
    
    int ret;
    
    // Initialize mutex
    ret = pthread_mutex_init(&barrier->mutex, NULL);
    if (ret != 0) {
        return ret;
    }
    
    // Initialize condition variable
    ret = pthread_cond_init(&barrier->cond, NULL);
    if (ret != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return ret;
    }
    
    // Initialize barrier state
    barrier->count = 0;
    barrier->trip_count = count;
    barrier->cycle = 0;  // Start at cycle 0
    
    return 0;
}

/**
 * Wait on a barrier
 * 
 * Blocks the calling thread until the required number of threads have called wait
 * 
 * @param barrier Pointer to barrier structure
 * @return 0 on success, error code on failure, PTHREAD_BARRIER_SERIAL_THREAD for the releasing thread
 */
int my_barrier_wait(my_barrier_t *barrier) {
    if (barrier == NULL) {
        return EINVAL;
    }
    
    int ret = 0;
    int my_cycle;
    int is_serial_thread = 0;
    
    // Lock the mutex
    ret = pthread_mutex_lock(&barrier->mutex);
    if (ret != 0) {
        return ret;
    }
    
    // Get current cycle to check against when waiting
    my_cycle = barrier->cycle;
    
    // Increment the count of threads at the barrier
    barrier->count++;
    
    if (barrier->count >= barrier->trip_count) {
        // This is the last thread to reach the barrier
        // Reset the count for the next use of the barrier
        barrier->count = 0;
        // Increment the cycle to avoid spurious wakeups
        barrier->cycle++;
        // Signal all waiting threads
        ret = pthread_cond_broadcast(&barrier->cond);
        // This thread is the "serial" thread
        is_serial_thread = 1;
    } else {
        // Not the last thread, wait for others
        while (barrier->cycle == my_cycle) {
            ret = pthread_cond_wait(&barrier->cond, &barrier->mutex);
            if (ret != 0) {
                break;
            }
        }
    }
    
    // Unlock the mutex
    int unlock_ret = pthread_mutex_unlock(&barrier->mutex);
    
    // Return the error code from wait if there was one, otherwise from unlock
    if (ret != 0) {
        return ret;
    }
    if (unlock_ret != 0) {
        return unlock_ret;
    }
    
    // Return special value for the thread that released the barrier
    return is_serial_thread ? PTHREAD_BARRIER_SERIAL_THREAD : 0;
}

/**
 * Destroy a barrier
 * 
 * @param barrier Pointer to barrier structure
 * @return 0 on success, error code on failure
 */
int my_barrier_destroy(my_barrier_t *barrier) {
    if (barrier == NULL) {
        return EINVAL;
    }
    
    int ret;
    
    // Destroy the mutex
    ret = pthread_mutex_destroy(&barrier->mutex);
    if (ret != 0) {
        return ret;
    }
    
    // Destroy the condition variable
    ret = pthread_cond_destroy(&barrier->cond);
    if (ret != 0) {
        return ret;
    }
    
    return 0;
}
