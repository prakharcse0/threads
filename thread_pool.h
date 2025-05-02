#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>

/**
 * Task structure - represents a function to be executed by the thread pool
 */
typedef struct {
    void (*function)(void *);  // Function pointer to task function
    void *argument;            // Argument to be passed to the function
} task_t;

/**
 * Thread pool structure
 */
typedef struct {
    task_t *queue;             // Task queue
    int queue_size;            // Size of queue
    int head;                  // Queue head
    int tail;                  // Queue tail
    int count;                 // Number of pending tasks

    pthread_t *threads;        // Worker threads
    int num_threads;           // Number of threads

    pthread_mutex_t lock;      // Mutex for queue access
    pthread_cond_t not_empty;  // Signal when queue is not empty
    pthread_cond_t not_full;   // Signal when queue is not full

    bool shutdown;             // Shutdown flag
} threadpool_t;

/**
 * Create a new thread pool
 *
 * @param num_threads Number of worker threads to create
 * @param queue_size Size of the task queue
 * @return Pointer to new thread pool, or NULL on error
 */
threadpool_t *threadpool_create(int num_threads, int queue_size);

/**
 * Add a task to the thread pool
 *
 * @param pool Thread pool to add task to
 * @param function Function to execute
 * @param argument Argument to pass to function
 * @return 0 on success, -1 on error
 */
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument);

/**
 * Destroy the thread pool
 *
 * @param pool Thread pool to destroy
 * @return 0 on success, -1 on error
 */
int threadpool_destroy(threadpool_t *pool);

/**
 * Get the number of pending tasks in the queue
 *
 * @param pool Thread pool to query
 * @return Number of pending tasks, or -1 on error
 */
int threadpool_pending_tasks(threadpool_t *pool);

#endif /* THREAD_POOL_H */
