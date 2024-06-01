#include "manager.h"
#include "myutil.h"

#include <stdio.h>
#include <unistd.h>

void* manager(void* arg) {
    ManagerArguments* args = (ManagerArguments*) arg;
    int cookThreadPoolSize = args->cookThreadPoolSize;
    int deliveryThreadPoolSize = args->deliveryThreadPoolSize;
    int deliverySpeed = args->deliverySpeed;
    int socketFd = args->socketFd;
    int orderPipe[2] = {args->orderPipe[0], args->orderPipe[1]};

    printf("Manager thread started\n");
    printf("Cook thread pool size: %d\n", cookThreadPoolSize);
    printf("Delivery thread pool size: %d\n", deliveryThreadPoolSize);
    printf("Delivery speed: %d\n", deliverySpeed);
    printf("Socket file descriptor: %d\n", socketFd);
    printf("Order pipe read end: %d\n", orderPipe[READ_END_PIPE]);
    printf("Order pipe write end: %d\n", orderPipe[WRITE_END_PIPE]);

    return NULL;
}