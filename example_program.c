#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "my_semaphore.h"
#include "my_recursive_mutex.h"
#include "my_condition_variable.h"
#include "thread_pool.h"
#include "my_barrier.h"

#define NUM_THREADS 4
#define NUM_TASKS 10
#define BARRIER_COUNT 3

// Global variables for examples
my_sem_t binary_sem;
my_recursive_mutex_t rec_mutex;
my_cond_t cond;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_value = 0;
my_barrier_t barrier;

// Example task for thread pool
void example_task(void *arg) {
    int task_id = *(int*)arg;
    printf("Thread %lu executing task %d\n", pthread_self(), task_id);
    
    // Simulate some work
    usleep(100000 + (rand() % 500000));
    printf("Task %d completed\n", task_id);
}

// Example demonstrating recursive mutex
void *recursive_mutex_demo(void *arg) {
    int thread_id = *(int*)arg;
    
    printf("Thread %d: Attempting to lock recursive mutex\n", thread_id);
    my_recursive_mutex_lock(&rec_mutex);
    printf("Thread %d: Got first lock\n", thread_id);
    
    // Acquire the same mutex again - would deadlock with a regular mutex
    my_recursive_mutex_lock(&rec_mutex);
    printf("Thread %d: Got second lock (recursive)\n", thread_id);
    
    // Do some work
    usleep(200000);
    
    // Release the locks in reverse order
    my_recursive_mutex_unlock(&rec_mutex);
    printf("Thread %d: Released second lock\n", thread_id);
    
    my_recursive_mutex_unlock(&rec_mutex);
    printf("Thread %d: Released first lock\n", thread_id);
    
    return NULL;
}

// Example demonstrating condition variables
void *condition_producer(void *arg) {
    int produced_items = 0;
    
    while (produced_items < 5) {
        // Sleep to simulate work
        usleep(500000);
        
        // Acquire mutex and modify shared data
        pthread_mutex_lock(&cond_mutex);
        
        // Update shared value
        shared_value++;
        produced_items++;
        
        printf("Producer: value set to %d\n", shared_value);
        
        // Signal waiting consumers
        my_cond_signal(&cond);
        
        pthread_mutex_unlock(&cond_mutex);
    }
    
    return NULL;
}

void *condition_consumer(void *arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < 2; i++) {
        // Acquire mutex
        pthread_mutex_lock(&cond_mutex);
        
        // Wait for a value change
        printf("Consumer %d: waiting for value change\n", thread_id);
        while (shared_value == 0) {
            my_cond_wait(&cond, &cond_mutex);
        }
        
        // Consume the value
        printf("Consumer %d: received value %d\n", thread_id, shared_value);
        shared_value = 0;  // Reset for next round
        
        pthread_mutex_unlock(&cond_mutex);
    }
    
    return NULL;
}

// Example demonstrating semaphores
void *semaphore_demo(void *arg) {
    int thread_id = *(int*)arg;
    
    printf("Thread %d: Waiting on semaphore\n", thread_id);
    my_sem_wait(&binary_sem);
    
    printf("Thread %d: Got the semaphore, working...\n", thread_id);
    // Critical section
    usleep(1000000);  // Simulate work for 1 second
    
    printf("Thread %d: Releasing semaphore\n", thread_id);
    my_sem_post(&binary_sem);
    
    return NULL;
}

// Example demonstrating barriers
void *barrier_demo(void *arg) {
    int thread_id = *(int*)arg;
    
    for (int phase = 1; phase <= 3; phase++) {
        // Do some work for this phase
        printf("Thread %d: Working on phase %d\n", thread_id, phase);
        usleep(200000 + (rand() % 800000));  // Random work time
        
        printf("Thread %d: Completed phase %d, waiting at barrier\n", thread_id, phase);
        
        // Wait at the barrier for all threads to finish this phase
        int result = my_barrier_wait(&barrier);
        
        // The "serial" thread (one that released the barrier) can perform special actions
        if (result == PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("---- All threads completed phase %d ----\n", phase);
        }
    }
    
    return NULL;
}

int main() {
    printf("Thread Synchronization Mechanisms Demo\n");
    printf("======================================\n\n");
    
    // Initialize random seed
    srand(time(NULL));
    
    // ------------ SEMAPHORE EXAMPLE ------------
    printf("\n--- Semaphore Example ---\n");
    my_sem_init(&binary_sem, 0, 1);  // Binary semaphore
    
    pthread_t sem_threads[3];
    int thread_ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&sem_threads[i], NULL, semaphore_demo, &thread_ids[i]);
    }
    
    // Wait for semaphore threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(sem_threads[i], NULL);
    }
    
    my_sem_destroy(&binary_sem);
    
    // ------------ RECURSIVE MUTEX EXAMPLE ------------
    printf("\n--- Recursive Mutex Example ---\n");
    my_recursive_mutex_init(&rec_mutex);
    
    pthread_t rec_thread;
    pthread_create(&rec_thread, NULL, recursive_mutex_demo, &thread_ids[0]);
    pthread_join(rec_thread, NULL);
    
    my_recursive_mutex_destroy(&rec_mutex);
    
    // ------------ CONDITION VARIABLE EXAMPLE ------------
    printf("\n--- Condition Variable Example ---\n");
    my_cond_init(&cond);
    
    pthread_t producer, consumers[2];
    
    // Create consumer threads
    for (int i = 0; i < 2; i++) {
        pthread_create(&consumers[i], NULL, condition_consumer, &thread_ids[i]);
    }
    
    // Create producer thread
    pthread_create(&producer, NULL, condition_producer, NULL);
    
    // Wait for all threads
    pthread_join(producer, NULL);
    for (int i = 0; i < 2; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    my_cond_destroy(&cond);
    
    // ------------ BARRIER EXAMPLE ------------
    printf("\n--- Barrier Example ---\n");
    my_barrier_init(&barrier, BARRIER_COUNT);
    
    pthread_t barrier_threads[BARRIER_COUNT];
    
    for (int i = 0; i < BARRIER_COUNT; i++) {
        pthread_create(&barrier_threads[i], NULL, barrier_demo, &thread_ids[i]);
    }
    
    // Wait for all barrier threads to complete
    for (int i = 0; i < BARRIER_COUNT; i++) {
        pthread_join(barrier_threads[i], NULL);
    }
    
    my_barrier_destroy(&barrier);
    
    // ------------ THREAD POOL EXAMPLE ------------
    printf("\n--- Thread Pool Example ---\n");
    
    // Create thread pool with 4 worker threads and a queue size of 10
    threadpool_t *pool = threadpool_create(NUM_THREADS, NUM_TASKS);
    
    if (pool == NULL) {
        fprintf(stderr, "Failed to create thread pool\n");
        return 1;
    }
    
    // Create task data
    int *task_args = (int*)malloc(NUM_TASKS * sizeof(int));
    
    // Add tasks to the pool
    for (int i = 0; i < NUM_TASKS; i++) {
        task_args[i] = i;
        
        if (threadpool_add_task(pool, example_task, &task_args[i]) != 0) {
            fprintf(stderr, "Failed to add task %d to thread pool\n", i);
        } else {
            printf("Added task %d to thread pool\n", i);
        }
    }
    
    // Allow time for tasks to execute
    printf("Waiting for all tasks to complete...\n");
    sleep(3);
    
    // Cleanup thread pool
    threadpool_destroy(pool);
    free(task_args);
    
    printf("\nAll synchronization examples completed successfully!\n");
    
    return 0;
}
