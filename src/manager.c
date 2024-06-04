#include "manager.h"
#include "myutil.h"
#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

void* manager(void* arg) {
    struct ManagerArguments* args = (struct ManagerArguments*) arg;
    int cookThreadPoolSize = args->cookThreadPoolSize;
    int deliveryThreadPoolSize = args->deliveryThreadPoolSize;
    int deliverySpeed = args->deliverySpeed;
    int socketFd = args->socketFd;
    int orderPipe[2] = {args->orderPipe[0], args->orderPipe[1]};
    enum TerminationCondition* terminationCondition = args->terminationCondition;
    pthread_cond_t* managerWorkCond = args->managerWorkCond;
    pthread_mutex_t* managerWorkMutex = args->managerWorkMutex;

    // In a loop read from the pipe and print the result after that close the client socket
    struct Order order;
    int readBytes = 0;
    while (1) {
        pthread_mutex_lock(managerWorkMutex);
        // Avoid spurious wakeups
        while (*terminationCondition != SERVER_SHUTDOWN && pthread_cond_wait(managerWorkCond, managerWorkMutex) != 0);
        if (*terminationCondition == SERVER_SHUTDOWN) {
            pthread_mutex_unlock(managerWorkMutex);
            break;
        }
        NO_EINTR(readBytes = read(orderPipe[READ_END_PIPE], &order, sizeof(struct Order)));
        if (readBytes == -1) {
            errExit("read");
        }
        logAndPrintMessage("Order received from client: %d %d %d %d\n", order.numberOfClients, order.width, order.height, order.clientSocketFd);
        close(order.clientSocketFd);
        pthread_mutex_unlock(managerWorkMutex);
    }

    return NULL;
}