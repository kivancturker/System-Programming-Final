#include "manager.h"
#include "myutil.h"
#include "logger.h"
#include "cook.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

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

    // Seed the random variable once
    srand(time(NULL));

    int mealOrderPipe[2];
    if (pipe(mealOrderPipe) == -1) {
        errExit("pipe");
    }

    int mealCompletePipe[2];
    if (pipe(mealCompletePipe) == -1) {
        errExit("pipe");
    }

    pthread_mutex_t mealOrderPipeMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mealCompletePipeMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cookWorkCond = PTHREAD_COND_INITIALIZER;

    // Create the cook threads
    struct CookArguments cookArgs = {
        .mealOrderPipe = {mealOrderPipe[0], mealOrderPipe[1]},
        .mealCompletePipe = {mealCompletePipe[0], mealCompletePipe[1]},
        .managerWorkMutex = managerWorkMutex,
        .managerWorkCond = managerWorkCond,
        .cookWorkCond = &cookWorkCond,
        .mealOrderPipeMutex = &mealOrderPipeMutex,
        .mealCompletePipeMutex = &mealCompletePipeMutex
    };
    pthread_t cookThreads[cookThreadPoolSize];
    createThreadPool(cookThreads, cookThreadPoolSize, cook, &cookArgs);

    // In a loop read from the pipe and print the result after that close the client socket
    struct Order order;
    int readBytes = 0;
    int writeBytes = 0;
    int completedMeals = 0;
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
        struct Coord* coords = generateRandomCoord(order.width, order.height, order.numberOfClients);
        struct Matrix* matrices = generateMatrix(order.numberOfClients);
        struct Meal *meals = (struct Meal*) malloc(order.numberOfClients * sizeof(struct Meal));
        for (int i = 0; i < order.numberOfClients; i++) {
            meals[i].matrix = matrices[i];
            meals[i].coord = coords[i];
        }

        // Later use the coordinates for delivery person
        for (int i = 0; i < order.numberOfClients; i++) {
            NO_EINTR(writeBytes = write(mealOrderPipe[WRITE_END_PIPE], &meals[i], sizeof(struct Meal)));
            if (writeBytes == -1) {
                errExit("write");
            }
        }

        for (int i = 0; i < order.numberOfClients; i++) {
            NO_EINTR(readBytes = read(mealCompletePipe[READ_END_PIPE], &meals[i], sizeof(struct Meal)));
            if (readBytes == -1) {
                errExit("read");
            }
            printf("Time takken for meal %d: %ld and meal is %d\n", i, meals[i].timeTaken, meals[i].matrix.number);
        }

        close(order.clientSocketFd);
        free(coords);
        free(matrices);
        pthread_mutex_unlock(managerWorkMutex);
    }

    if (*terminationCondition == SERVER_SHUTDOWN) {
        cancelThreadPool(cookThreads, cookThreadPoolSize);
    }

    joinThreadPool(cookThreads, cookThreadPoolSize);

    close(mealOrderPipe[READ_END_PIPE]);
    close(mealOrderPipe[WRITE_END_PIPE]);
    close(mealCompletePipe[READ_END_PIPE]);
    close(mealCompletePipe[WRITE_END_PIPE]);

    return NULL;
}