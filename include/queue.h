#ifndef QUEUE_H
#define QUEUE_H

#include "myutil.h"

#include <pthread.h>

struct Queue {
    int frontIndex;
    int rearIndex;
    int size;
    int capacity;
    pthread_t* threads;
};

int initQueue(struct Queue* queue, int capacity);
int enqueue(struct Queue* queue, pthread_t thread);
int dequeue(struct Queue* queue, pthread_t* thread);
int isQueueEmpty(struct Queue* queue);
void destroyQueue(struct Queue* queue);

#endif // QUEUE_H