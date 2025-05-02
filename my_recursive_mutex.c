#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "my_recursive_mutex.h"

/**
 * Initialize our recursive mutex
 * 
 * @param rmutex Pointer to the recursive mutex to initialize
 * @return 0 on success, error code on failure
 */
int my_recursive_mutex_init(my_recursive_mutex_t* rmutex) {
    if (rmutex == NULL) {
        return EINVAL;
    }
    
    // Initialize the internal state
    rmutex->lock_count = 0;
    rmutex->initialized = 1;
    
    // Initialize the underlying mutex
    return pthread_mutex_init(&rmutex->mutex, NULL);
}

/**
 * Lock our recursive mutex
 * 
 * If the current thread already owns the lock, just increment the lock count.
 * Otherwise, acquire the underlying mutex.
 * 
 * @param rmutex Pointer to the recursive mutex
 * @return 0 on success, error code on failure
 */
int my_recursive_mutex_lock(my_recursive_mutex_t* rmutex) {
    if (rmutex == NULL || !rmutex->initialized) {
        return EINVAL;
    }
    
    // Get current thread ID
    pthread_t self = pthread_self();

    // Check if we already own the lock
    if (rmutex->lock_count > 0 && pthread_equal(rmutex->owner, self)) {
        // We already own the lock, just increment the count
        rmutex->lock_count++;
        return 0;
    }

    // We don't own the lock, acquire the internal mutex
    int result = pthread_mutex_lock(&rmutex->mutex);
    if (result == 0) {
        // We now own the lock
        rmutex->owner = self;
        rmutex->lock_count = 1;
    }
    return result;
}

/**
 * Try to lock our recursive mutex without blocking
 * 
 * Same as lock, but returns immediately with an error if the mutex
 * is already locked by another thread.
 * 
 * @param rmutex Pointer to the recursive mutex
 * @return 0 on success, EBUSY if mutex is locked by another thread, other error codes on failure
 */
int my_recursive_mutex_trylock(my_recursive_mutex_t* rmutex) {
    if (rmutex == NULL || !rmutex->initialized) {
        return EINVAL;
    }
    
    // Get current thread ID
    pthread_t self = pthread_self();

    // Check if we already own the lock
    if (rmutex->lock_count > 0 && pthread_equal(rmutex->owner, self)) {
        // We already own the lock, just increment the count
        rmutex->lock_count++;
        return 0;
    }

    // We don't own the lock, try to acquire the internal mutex
    int result = pthread_mutex_trylock(&rmutex->mutex);
    if (result == 0) {
        // We now own the lock
        rmutex->owner = self;
        rmutex->lock_count = 1;
    }
    return result;
}

/**
 * Unlock our recursive mutex
 * 
 * Decrements the lock count. Only releases the actual mutex
 * when the count reaches zero.
 * 
 * @param rmutex Pointer to the recursive mutex
 * @return 0 on success, error code on failure
 */
int my_recursive_mutex_unlock(my_recursive_mutex_t* rmutex) {
    if (rmutex == NULL || !rmutex->initialized) {
        return EINVAL;
    }
    
    // Get current thread ID
    pthread_t self = pthread_self();

    // Make sure we're the owner
    if (!pthread_equal(rmutex->owner, self)) {
        return EPERM; // Not owner, return error
    }

    // Decrement lock count
    rmutex->lock_count--;

    // If count reaches zero, release the real mutex
    if (rmutex->lock_count == 0) {
        return pthread_mutex_unlock(&rmutex->mutex);
    }

    return 0;
}

/**
 * Get the current lock count of the recursive mutex
 * 
 * @param rmutex Pointer to the recursive mutex
 * @return The lock count, or -1 on error
 */
int my_recursive_mutex_get_count(my_recursive_mutex_t* rmutex) {
    if (rmutex == NULL || !rmutex->initialized) {
        return -1;
    }
    
    // Get current thread ID
    pthread_t self = pthread_self();
    
    // If we're the owner, return the count
    if (rmutex->lock_count > 0 && pthread_equal(rmutex->owner, self)) {
        return rmutex->lock_count;
    }
    
    // If we're not the owner, return 0
    return 0;
}

/**
 * Destroy our recursive mutex
 * 
 * Only succeeds if the mutex is not currently locked.
 * 
 * @param rmutex Pointer to the recursive mutex
 * @return 0 on success, error code on failure
 */
int my_recursive_mutex_destroy(my_recursive_mutex_t* rmutex) {
    if (rmutex == NULL || !rmutex->initialized) {
        return EINVAL;
    }

    if (rmutex->lock_count != 0) {
        return EBUSY; // Still locked, can't destroy
    }

    rmutex->initialized = 0;
    return pthread_mutex_destroy(&rmutex->mutex);
}
