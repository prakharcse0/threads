#ifndef MY_RECURSIVE_MUTEX_H
#define MY_RECURSIVE_MUTEX_H

#include <pthread.h>
#include <errno.h>

/**
 * MyRecursiveMutex - A custom implementation of a recursive mutex
 * 
 * A recursive mutex allows the same thread to lock it multiple times without deadlock.
 * It works by tracking:
 * 1. Which thread currently owns the lock
 * 2. How many times the thread has locked it
 * 3. Using a regular mutex to protect this tracking information
 */
typedef struct {
    pthread_mutex_t mutex;   // Regular mutex to protect our structure
    pthread_t owner;         // ID of thread that currently owns the lock
    int lock_count;          // Number of times the owner has locked it
    int initialized;         // Flag to check if the mutex is initialized
} my_recursive_mutex_t;

// Function prototypes
int my_recursive_mutex_init(my_recursive_mutex_t* rmutex);
int my_recursive_mutex_lock(my_recursive_mutex_t* rmutex);
int my_recursive_mutex_unlock(my_recursive_mutex_t* rmutex);
int my_recursive_mutex_destroy(my_recursive_mutex_t* rmutex);

// Additional helper functions
int my_recursive_mutex_trylock(my_recursive_mutex_t* rmutex);
int my_recursive_mutex_get_count(my_recursive_mutex_t* rmutex);

#endif /* MY_RECURSIVE_MUTEX_H */
