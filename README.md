# Thread Synchronization Mechanisms

A comprehensive implementation of various thread synchronization primitives in C, demonstrating low-level concurrency control mechanisms.

## Overview

This project implements several core thread synchronization mechanisms from scratch:

1. **Custom Semaphores**: Implementation of semaphore operations (sem_init, sem_wait, sem_post, sem_destroy) using mutexes and condition variables.
2. **Recursive Mutexes**: Custom implementation allowing the same thread to acquire a mutex multiple times without deadlock.
3. **Condition Variables**: Implementation of condition variable functionality (wait, signal, broadcast) using a linked list of waiting threads.
4. **Thread Pool**: A complete thread pool implementation with task queue and worker management.
5. **Barrier**: Implementation of thread barrier synchronization.

## Implementation Details

### Custom Semaphores

The semaphore implementation (`my_semaphore.c`) provides:
- **sem_init**: Initialize semaphore with a starting value
- **sem_wait**: Decrement semaphore (blocking if value is 0)
- **sem_post**: Increment semaphore and signal waiting threads
- **sem_destroy**: Clean up semaphore resources

The implementation uses pthread mutexes and condition variables to ensure atomic operations and efficient thread waiting.

### Recursive Mutexes

`my_recursive_mutex.c` implements a recursive mutex that:
- Tracks the current owner thread ID
- Maintains a lock count for recursive acquisitions
- Allows the same thread to lock the mutex multiple times
- Only releases the actual mutex when the lock count reaches zero

### Condition Variables

The condition variable implementation (`my_condition_variable.c`) provides:
- **my_cond_init**: Initialize the condition variable structure
- **my_cond_wait**: Wait on a condition (atomically releasing the mutex)
- **my_cond_signal**: Wake up a single waiting thread
- **my_cond_broadcast**: Wake up all waiting threads

The implementation uses a linked list to track waiting threads and manages their state through a thread-safe mechanism.

### Thread Pool

`thread_pool.c` implements a complete thread pool with:
- Worker thread management
- Thread-safe task queue
- Dynamic task submission
- Clean shutdown capabilities

The thread pool efficiently distributes tasks among a fixed number of worker threads, providing a high-performance solution for concurrent task execution.

### Barrier

`my_barrier.c` provides a synchronization point for multiple threads:
- All threads wait at the barrier until a specified number have arrived
- When the count is reached, all threads are released simultaneously
- Barrier can be reused for multiple synchronization cycles

## Usage Examples

### Semaphores

```c
my_sem_t sem;
my_sem_init(&sem, 0, 1); // Initialize binary semaphore

// Thread 1
my_sem_wait(&sem);   // Acquire the semaphore
// Critical section
my_sem_post(&sem);   // Release the semaphore

// Cleanup
my_sem_destroy(&sem);
```

### Recursive Mutex

```c
MyRecursiveMutex rmutex;
my_recursive_mutex_init(&rmutex);

// Same thread can acquire multiple times
my_recursive_mutex_lock(&rmutex);
my_recursive_mutex_lock(&rmutex);  // No deadlock!
// Critical section
my_recursive_mutex_unlock(&rmutex);
my_recursive_mutex_unlock(&rmutex);

my_recursive_mutex_destroy(&rmutex);
```

### Condition Variables

```c
my_cond_t cond;
my_mutex_t mutex;
my_cond_init(&cond);
pthread_mutex_init(&mutex, NULL);

// Thread 1 - Waiter
pthread_mutex_lock(&mutex);
while (!condition_met) {
    my_cond_wait(&cond, &mutex);
}
pthread_mutex_unlock(&mutex);

// Thread 2 - Signaler
pthread_mutex_lock(&mutex);
condition_met = true;
my_cond_signal(&cond);
pthread_mutex_unlock(&mutex);
```

### Thread Pool

```c
void task_function(void *arg) {
    int *num = (int*)arg;
    printf("Processing task: %d\n", *num);
}

int main() {
    threadpool_t *pool = threadpool_create(4, 10);  // 4 threads, queue size 10
    
    int task_args[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        threadpool_add_task(pool, task_function, &task_args[i]);
    }
    
    sleep(1);  // Allow tasks to complete
    threadpool_destroy(pool);
    return 0;
}
```

### Barrier

```c
my_barrier_t barrier;
my_barrier_init(&barrier, 3);  // Wait for 3 threads

// In each thread:
// ... do first part of work ...
my_barrier_wait(&barrier);  // Synchronize here
// ... do second part of work ...
```

## Building and Testing

```bash
# Compile all implementations
make all

# Run tests
make test

# Clean build files
make clean
```

