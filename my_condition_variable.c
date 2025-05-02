#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "my_condition_variable.h"

/* Initialize a condition variable */
void my_cond_init(my_cond_t* cond) {
    pthread_mutex_init(&cond->internal_mutex, NULL);
    cond->wait_list = NULL;
}

/* Destroy a condition variable */
void my_cond_destroy(my_cond_t* cond) {
    pthread_mutex_lock(&cond->internal_mutex);
    
    // Free all nodes in the wait list
    waiting_thread_t* current = cond->wait_list;
    waiting_thread_t* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    pthread_mutex_unlock(&cond->internal_mutex);
    pthread_mutex_destroy(&cond->internal_mutex);
}

/*
 * Wait on a condition variable
 * This is the core of the condition variable mechanism
 */
void my_cond_wait(my_cond_t* cond, my_mutex_t* mutex) {
    // Create a node for the current thread
    waiting_thread_t* node = malloc(sizeof(waiting_thread_t));
    if (node == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Failed to allocate memory for waiting thread\n");
        return;
    }
    
    node->thread_id = pthread_self();
    node->is_awakened = false;
    node->next = NULL;

    // Add this thread to the wait list (protected by internal_mutex)
    pthread_mutex_lock(&cond->internal_mutex);
    if (cond->wait_list == NULL) {
        cond->wait_list = node;
    } else {
        // Add to the end of the list
        waiting_thread_t* current = cond->wait_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = node;
    }
    pthread_mutex_unlock(&cond->internal_mutex);

    // Release the external mutex (the one provided by the caller)
    // This is the critical step that allows other threads to proceed
    pthread_mutex_unlock(mutex);

    // Busy-wait until this thread is awakened
    // In a real implementation, this would use thread suspension mechanisms
    // such as futexes, semaphores, or other OS primitives
    while (1) {
        pthread_mutex_lock(&cond->internal_mutex);
        bool awakened = node->is_awakened;
        pthread_mutex_unlock(&cond->internal_mutex);

        if (awakened) {
            break;
        }

        // Yield to other threads and reduce CPU usage
        // Real implementations would put the thread to sleep
        usleep(1000);
    }

    // Once awakened, reacquire the external mutex before proceeding
    // This ensures the mutex is held when returning to the caller
    pthread_mutex_lock(mutex);

    // Clean up the node
    pthread_mutex_lock(&cond->internal_mutex);
    
    // Remove self from list
    if (cond->wait_list != NULL) {
        if (pthread_equal(cond->wait_list->thread_id, node->thread_id)) {
            // First node is our node
            waiting_thread_t* temp = cond->wait_list;
            cond->wait_list = cond->wait_list->next;
            // Don't free 'temp' here as it's the same as 'node'
        } else {
            // Search for our node in the list
            waiting_thread_t* current = cond->wait_list;
            while (current->next != NULL) {
                if (pthread_equal(current->next->thread_id, node->thread_id)) {
                    waiting_thread_t* temp = current->next;
                    current->next = temp->next;
                    // Don't free 'temp' here as it's the same as 'node'
                    break;
                }
                current = current->next;
            }
        }
    }
    
    pthread_mutex_unlock(&cond->internal_mutex);
    free(node);
}

/* Signal (wake up) one waiting thread */
void my_cond_signal(my_cond_t* cond) {
    pthread_mutex_lock(&cond->internal_mutex);

    // Find the first waiting thread
    waiting_thread_t* thread = cond->wait_list;
    if (thread != NULL) {
        // Mark it as awakened
        thread->is_awakened = true;
    }

    pthread_mutex_unlock(&cond->internal_mutex);
}

/* Broadcast (wake up) all waiting threads */
void my_cond_broadcast(my_cond_t* cond) {
    pthread_mutex_lock(&cond->internal_mutex);

    // Mark all threads as awakened
    waiting_thread_t* thread = cond->wait_list;
    while (thread != NULL) {
        thread->is_awakened = true;
        thread = thread->next;
    }

    pthread_mutex_unlock(&cond->internal_mutex);
}
