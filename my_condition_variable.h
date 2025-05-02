#ifndef MY_CONDITION_VARIABLE_H
#define MY_CONDITION_VARIABLE_H

#include <pthread.h>
#include <stdbool.h>

/* Define a structure for a waiting thread */
typedef struct waiting_thread {
    pthread_t thread_id;         // ID of the waiting thread
    struct waiting_thread* next; // Pointer to next waiting thread
    bool is_awakened;           // Flag to indicate if thread has been signaled
} waiting_thread_t;

/* Define our condition variable structure */
typedef struct my_cond {
    pthread_mutex_t internal_mutex; // Internal mutex to protect condition variable state
    waiting_thread_t* wait_list;    // Linked list of waiting threads
} my_cond_t;

/* Define our mutex wrapper for clarity */
typedef pthread_mutex_t my_mutex_t;

/* Function prototypes */
void my_cond_init(my_cond_t* cond);
void my_cond_destroy(my_cond_t* cond);
void my_cond_wait(my_cond_t* cond, my_mutex_t* mutex);
void my_cond_signal(my_cond_t* cond);
void my_cond_broadcast(my_cond_t* cond);

#endif /* MY_CONDITION_VARIABLE_H */
