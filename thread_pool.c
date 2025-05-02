#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "thread_pool.h"

/**
 * Worker thread function - pulls tasks from the queue and executes them
 */
static void *worker(void *arg) {
    threadpool_t *pool = (threadpool_t *)arg;
    task_t task;

    while (1) {
        // Lock the mutex
        pthread_mutex_lock(&(pool->lock));

        // Wait for work if queue is empty
        while (pool->count == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->not_empty), &(pool->lock));
        }

        // Check for shutdown
        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        // Get task from queue
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;

        // Update queue
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count--;

        // Signal that the queue is not full
        pthread_cond_signal(&(pool->not_full));

        // Unlock
        pthread_mutex_unlock(&(pool->lock));

        // Execute task
        (*(task.function))(task.argument);
    }

    return NULL;
}

/**
 * Create a new thread pool
 *
 * @param num_threads Number of worker threads to create
 * @param queue_size Size of the task queue
 * @return Pointer to new thread pool, or NULL on error
 */
threadpool_t *threadpool_create(int num_threads, int queue_size) {
    if (num_threads <= 0 || queue_size <= 0) {
        errno = EINVAL;
        return NULL;
    }

    // Allocate the pool structure
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if (pool == NULL) {
        return NULL;
    }

    // Initialize members
    pool->queue_size = queue_size;
    pool->num_threads = num_threads;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = false;

    // Allocate memory for threads and task queue
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    pool->queue = (task_t *)malloc(sizeof(task_t) * queue_size);

    if (pool->threads == NULL || pool->queue == NULL) {
        free(pool->threads);
        free(pool->queue);
        free(pool);
        return NULL;
    }

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->not_empty), NULL) != 0 ||
        pthread_cond_init(&(pool->not_full), NULL) != 0) {
        
        free(pool->threads);
        free(pool->queue);
        free(pool);
        return NULL;
    }

    // Start worker threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, worker, (void *)pool) != 0) {
            // Clean up on error
            threadpool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

/**
 * Add a task to the thread pool
 *
 * @param pool Thread pool to add task to
 * @param function Function to execute
 * @param argument Argument to pass to function
 * @return 0 on success, -1 on error
 */
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument) {
    if (pool == NULL || function == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Lock mutex
    if (pthread_mutex_lock(&(pool->lock)) != 0) {
        return -1;
    }

    // Wait if queue is full
    while (pool->count == pool->queue_size && !pool->shutdown) {
        if (pthread_cond_wait(&(pool->not_full), &(pool->lock)) != 0) {
            pthread_mutex_unlock(&(pool->lock));
            return -1;
        }
    }

    // Don't add tasks if shutting down
    if (pool->shutdown) {
        pthread_mutex_unlock(&(pool->lock));
        errno = ESHUTDOWN;
        return -1;
    }

    // Add task to queue
    pool->queue[pool->tail].function = function;
    pool->queue[pool->tail].argument = argument;
    pool->tail = (pool->tail + 1) % pool->queue_size;
    pool->count++;

    // Signal that queue is not empty
    if (pthread_cond_signal(&(pool->not_empty)) != 0) {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    // Unlock
    if (pthread_mutex_unlock(&(pool->lock)) != 0) {
        return -1;
    }

    return 0;
}

/**
 * Get the number of pending tasks in the queue
 *
 * @param pool Thread pool to query
 * @return Number of pending tasks, or -1 on error
 */
int threadpool_pending_tasks(threadpool_t *pool) {
    if (pool == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    int count = 0;
    
    if (pthread_mutex_lock(&(pool->lock)) != 0) {
        return -1;
    }
    
    count = pool->count;
    
    if (pthread_mutex_unlock(&(pool->lock)) != 0) {
        return -1;
    }
    
    return count;
}

/**
 * Destroy the thread pool
 *
 * @param pool Thread pool to destroy
 * @return 0 on success, -1 on error
 */
int threadpool_destroy(threadpool_t *pool) {
    if (pool == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Lock mutex
    if (pthread_mutex_lock(&(pool->lock)) != 0) {
        return -1;
    }

    // Set shutdown flag
    pool->shutdown = true;

    // Wake up all threads
    if (pthread_cond_broadcast(&(pool->not_empty)) != 0 ||
        pthread_mutex_unlock(&(pool->lock)) != 0) {
        return -1;
    }

    // Join all worker threads
    for (int i = 0; i < pool->num_threads; i++) {
        if (pthread_join(pool->threads[i], NULL) != 0) {
            return -1;
        }
    }

    // Free resources
    free(pool->threads);
    free(pool->queue);

    // Clean up synchronization objects
    if (pthread_mutex_destroy(&(pool->lock)) != 0 ||
        pthread_cond_destroy(&(pool->not_empty)) != 0 ||
        pthread_cond_destroy(&(pool->not_full)) != 0) {
        return -1;
    }

    free(pool);
    return 0;
}
