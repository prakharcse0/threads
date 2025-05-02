#include <stdio.h>
#include <errno.h>
#include "my_semaphore.h"

/**
 * Initialize a semaphore with a specified value
 * 
 * @param sem Pointer to the semaphore to initialize
 * @param pshared Flag indicating if semaphore is shared between processes (0 for thread-only)
 * @param value Initial value for the semaphore
 * @return 0 on success, -1 on error
 */
int my_sem_init(my_sem_t *sem, int pshared, unsigned int value) {
    // Validate input parameters
    if (sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Initialize the mutex (the lock)
    if (pthread_mutex_init(&sem->lock, NULL) != 0) {
        return -1;
    }

    // Initialize the condition variable
    if (pthread_cond_init(&sem->cond, NULL) != 0) {
        pthread_mutex_destroy(&sem->lock);
        return -1;
    }

    // Set the initial value of the semaphore
    sem->count = value;
    sem->waiting = 0;

    // For now, we're ignoring the pshared parameter
    // A complete implementation would handle process-shared semaphores differently

    return 0; // Success
}

/**
 * Wait on a semaphore (decrement or block)
 * 
 * If the semaphore value is greater than zero, decrements it and returns.
 * If the value is zero, blocks until it becomes greater than zero.
 * 
 * @param sem Pointer to the semaphore
 * @return 0 on success, -1 on error
 */
int my_sem_wait(my_sem_t *sem) {
    if (sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    // First, acquire the lock to protect the semaphore
    int ret = pthread_mutex_lock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    // Check if we need to wait
    while (sem->count == 0) {
        // No resources available, we need to wait
        sem->waiting++; // Track that we're waiting

        // Release the lock and wait on the condition variable
        // This atomically releases the mutex and blocks the thread
        ret = pthread_cond_wait(&sem->cond, &sem->lock);
        
        // When we wake up, the mutex is automatically reacquired
        sem->waiting--; // We're no longer waiting
        
        if (ret != 0) {
            pthread_mutex_unlock(&sem->lock);
            errno = ret;
            return -1;
        }
    }

    // At this point we know count > 0
    sem->count--; // Decrement the semaphore

    // Release the lock
    ret = pthread_mutex_unlock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    return 0; // Success
}

/**
 * Try to wait on a semaphore without blocking
 * 
 * If the semaphore value is greater than zero, decrements it and returns.
 * If the value is zero, returns immediately with an error.
 * 
 * @param sem Pointer to the semaphore
 * @return 0 on success, -1 on error with errno set
 */
int my_sem_trywait(my_sem_t *sem) {
    if (sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Acquire the lock
    int ret = pthread_mutex_lock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    // Check if resources are available
    if (sem->count == 0) {
        // No resources, would block, so return error
        pthread_mutex_unlock(&sem->lock);
        errno = EAGAIN;
        return -1;
    }

    // Resource available, decrement and return
    sem->count--;

    // Release the lock
    ret = pthread_mutex_unlock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    return 0; // Success
}

/**
 * Post to a semaphore (increment it)
 * 
 * Increments the semaphore value and if there are waiting threads,
 * wakes one of them up.
 * 
 * @param sem Pointer to the semaphore
 * @return 0 on success, -1 on error
 */
int my_sem_post(my_sem_t *sem) {
    if (sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    // First, acquire the lock
    int ret = pthread_mutex_lock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    // Increment the semaphore
    sem->count++;

    // If threads are waiting, wake one up
    if (sem->waiting > 0) {
        ret = pthread_cond_signal(&sem->cond);
        if (ret != 0) {
            pthread_mutex_unlock(&sem->lock);
            errno = ret;
            return -1;
        }
    }

    // Release the lock
    ret = pthread_mutex_unlock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    return 0; // Success
}

/**
 * Get the current value of the semaphore
 * 
 * @param sem Pointer to the semaphore
 * @param sval Pointer to store the semaphore value
 * @return 0 on success, -1 on error
 */
int my_sem_getvalue(my_sem_t *sem, int *sval) {
    if (sem == NULL || sval == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Acquire the lock
    int ret = pthread_mutex_lock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    // Get the value
    *sval = sem->count;

    // Release the lock
    ret = pthread_mutex_unlock(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    return 0;
}

/**
 * Destroy a semaphore
 * 
 * @param sem Pointer to the semaphore to destroy
 * @return 0 on success, -1 on error
 */
int my_sem_destroy(my_sem_t *sem) {
    if (sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Destroy the mutex and condition variable
    int ret = pthread_mutex_destroy(&sem->lock);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    ret = pthread_cond_destroy(&sem->cond);
    if (ret != 0) {
        errno = ret;
        return -1;
    }

    return 0; // Success
}
