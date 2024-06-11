#include "queue.h"

#include <stdlib.h>

// Initialize the queue
int initQueue(struct Queue* queue, int capacity) {
    if (queue == NULL || capacity <= 0) {
        return -1; // Invalid queue pointer or capacity
    }
    queue->threads = (pthread_t*)malloc(capacity * sizeof(pthread_t));
    if (queue->threads == NULL) {
        return -1; // Memory allocation failed
    }
    queue->frontIndex = 0;
    queue->rearIndex = 0;
    queue->size = 0;
    queue->capacity = capacity;
    return 0; // Queue initialized successfully
}

// Enqueue a connection request into the queue
int enqueue(struct Queue* queue, pthread_t thread) {
    if (queue == NULL || queue->threads == NULL) {
        return -1; // Invalid queue pointer
    }
    if (queue->size >= queue->capacity) {
        return -1; // Queue is full
    }
    queue->threads[queue->rearIndex] = thread;
    queue->rearIndex = (queue->rearIndex + 1) % queue->capacity;
    queue->size++;
    return 0; // Enqueue operation successful
}

// Dequeue a connection request from the queue
int dequeue(struct Queue* queue, pthread_t* thread) {
    if (queue == NULL || queue->threads == NULL || thread == NULL) {
        return -1; // Invalid queue or thread pointer
    }
    if (queue->size <= 0) {
        return -1; // Queue is empty
    }
    *thread = queue->threads[queue->frontIndex];
    queue->frontIndex = (queue->frontIndex + 1) % queue->capacity;
    queue->size--;
    return 0; // Dequeue operation successful
}

// Check if the queue is empty
int isQueueEmpty(struct Queue* queue) {
    if (queue == NULL) {
        return -1; // Invalid queue pointer
    }
    return (queue->size == 0);
}

void destroyQueue(struct Queue* queue) {
    if (queue != NULL) {
        free(queue->threads);
        queue->threads = NULL;
    }
}