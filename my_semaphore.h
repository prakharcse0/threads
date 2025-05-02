#ifndef MY_SEMAPHORE_H
#define MY_SEMAPHORE_H

#include <pthread.h>

typedef struct {
    unsigned int count;      // The current value of the semaphore
    pthread_mutex_t lock;    // A lock to ensure atomic operations
    pthread_cond_t cond;     // A condition variable for waiting threads
    int waiting;             // Number of threads currently waiting
} my_sem_t;

// Semaphore API functions
int my_sem_init(my_sem_t *sem, int pshared, unsigned int value);
int my_sem_wait(my_sem_t *sem);
int my_sem_post(my_sem_t *sem);
int my_sem_destroy(my_sem_t *sem);

// Helper function to attempt to acquire semaphore without blocking
int my_sem_trywait(my_sem_t *sem);

// Helper function to get the current semaphore value
int my_sem_getvalue(my_sem_t *sem, int *sval);

#endif /* MY_SEMAPHORE_H */
