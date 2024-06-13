#include "manager.h"
#include "myutil.h"
#include "logger.h"
#include "cook.h"
#include "delivery.h"
#include "queue.h"
#include "sockconn.h"

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
    // int socketFd = args->socketFd;
    int orderPipe[2] = {args->orderPipe[0], args->orderPipe[1]};
    enum TerminationCondition* terminationCondition = args->terminationCondition;
    pthread_cond_t* managerWorkCond = args->managerWorkCond;
    pthread_mutex_t* managerWorkMutex = args->managerWorkMutex;
    pthread_mutex_t* socketMutex = args->socketMutex;

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

    int mealToDeliverPipe[2];
    if (pipe(mealToDeliverPipe) == -1) {
        errExit("pipe");
    }

    pthread_cond_t cookWorkCond = PTHREAD_COND_INITIALIZER;
    pthread_cond_t deliveryCompletedCond = PTHREAD_COND_INITIALIZER;
    int deliveryCount = 0;
    pthread_mutex_t deliveryCountMutex = PTHREAD_MUTEX_INITIALIZER;

    struct Meal emptyMeal;

    struct Oven oven = {
        .meals = {emptyMeal, emptyMeal, emptyMeal, emptyMeal, emptyMeal, emptyMeal},
        .occupiedSpots = {0, 0, 0, 0, 0, 0},
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .aparatusCount = 3,
        .mealCount = 0,
        .ovenIsFull = PTHREAD_COND_INITIALIZER
    };

    // Create the cook threads
    struct CookArguments* cookArgs = (struct CookArguments*) malloc(cookThreadPoolSize * sizeof(struct CookArguments));
    for (int i = 0; i < cookThreadPoolSize; i++) {
        cookArgs[i].mealOrderPipe[0] = mealOrderPipe[0];
        cookArgs[i].mealOrderPipe[1] = mealOrderPipe[1];
        cookArgs[i].mealCompletePipe[0] = mealCompletePipe[0];
        cookArgs[i].mealCompletePipe[1] = mealCompletePipe[1];
        cookArgs[i].managerWorkMutex = managerWorkMutex;
        cookArgs[i].managerWorkCond = managerWorkCond;
        cookArgs[i].cookWorkCond = &cookWorkCond;
        cookArgs[i].oven = &oven;
        cookArgs[i].cookNum = i+1;
        cookArgs[i].socketMutex = socketMutex;
    }
    pthread_t cookThreads[cookThreadPoolSize];
    createCookThreadPool(cookThreads, cookThreadPoolSize, cook, cookArgs);

    struct DeliveryPersonArguments* deliveryPersonArgs = (struct DeliveryPersonArguments*) malloc(deliveryThreadPoolSize * sizeof(struct DeliveryPersonArguments));
    for (int i = 0; i < deliveryThreadPoolSize; i++) {
        deliveryPersonArgs[i].deliverySpeed = deliverySpeed;
        deliveryPersonArgs[i].mealToDeliverPipe[0] = mealToDeliverPipe[0];
        deliveryPersonArgs[i].mealToDeliverPipe[1] = mealToDeliverPipe[1];
        deliveryPersonArgs[i].deliveryPersonNum = i+1;
        deliveryPersonArgs[i].socketMutex = socketMutex;
        deliveryPersonArgs[i].deliveryCompletedCond = &deliveryCompletedCond;
        deliveryPersonArgs[i].deliveryCount = &deliveryCount;
        deliveryPersonArgs[i].deliveryCountMutex = &deliveryCountMutex;
    }
    pthread_t deliveryPersonThreads[deliveryThreadPoolSize];
    createDeliveryThreadPool(deliveryPersonThreads, deliveryThreadPoolSize, deliveryPerson, deliveryPersonArgs);

    // In a loop read from the pipe and print the result after that close the client socket
    struct Order order;
    int readBytes = 0;
    int writeBytes = 0;
    int completedMealsCount = 0;
    int expectedMeals = 0;
    int currentMealCountForDelivery = 0;
    struct MealToDeliver* mealsToDeliver;
    struct Meal* meals;
    struct MessagePacket messagePacket = {0};
    int mostEfficientCookNum = 0;
    int mostEfficientDeliveryPersonNum = 0;
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
            errExit("read from order pipe");
        }

        logAndPrintMessage("%d new customers... Serving\n", order.numberOfClients);
        expectedMeals = order.numberOfClients;
        meals = order.meals;

        mealsToDeliver = (struct MealToDeliver*) malloc(sizeof(struct MealToDeliver));
        mealsToDeliver->mealCount = 0;
        // Later use the coordinates for delivery person
        for (int i = 0; i < order.numberOfClients; i++) {
            // Put clientSocketFd in the meal struct
            meals[i].clientSocketFd = order.clientSocketFd;
            NO_EINTR(writeBytes = write(mealOrderPipe[WRITE_END_PIPE], &meals[i], sizeof(struct Meal)));
            if (writeBytes == -1) {
                errExit("write to meal order pipe");
            }
        }

        for (int i = 0; i < order.numberOfClients; i++) {
            NO_EINTR(readBytes = read(mealCompletePipe[READ_END_PIPE], &meals[i], sizeof(struct Meal)));
            if (readBytes == -1) {
                errExit("read from meal complete pipe");
            }
            completedMealsCount++;
            currentMealCountForDelivery++;

            // Send meals to the available delivery person
            // Either 3 meals or remaining meals then delivery person goes.
            mealsToDeliver->meal[mealsToDeliver->mealCount] = &meals[i];
            mealsToDeliver->mealCount++;
            mealsToDeliver->width = order.width;
            mealsToDeliver->height = order.height;
            if (currentMealCountForDelivery == 3 || completedMealsCount == expectedMeals) {
                struct MealToDeliver* tempMealsToDeliver = mealsToDeliver;
                writeBytes = write(mealToDeliverPipe[WRITE_END_PIPE], &tempMealsToDeliver, sizeof(struct MealToDeliver*));
                if (writeBytes == -1) {
                    errExit("write to meal to deliver pipe");
                }
                // Send the meals to the delivery person.
                currentMealCountForDelivery = 0;
                // After sending the meal reinitialize the mealsToDeliver
                mealsToDeliver = (struct MealToDeliver*) malloc(sizeof(struct MealToDeliver));
                mealsToDeliver->mealCount = 0;
                pthread_mutex_lock(&deliveryCountMutex);
                deliveryCount++;
                pthread_mutex_unlock(&deliveryCountMutex);
            }
        }

        pthread_mutex_lock(&deliveryCountMutex);
        while (deliveryCount != 0) {
            pthread_cond_wait(&deliveryCompletedCond, &deliveryCountMutex);
        }
        pthread_mutex_unlock(&deliveryCountMutex);
        completedMealsCount = 0;
        mostEfficientCookNum = getMostEfficientCook(meals, order.numberOfClients, cookThreadPoolSize);
        mostEfficientDeliveryPersonNum = getMostEfficientDeliveryPerson(meals, order.numberOfClients, deliveryThreadPoolSize);
        logAndPrintMessage("done serving client PID: %d\n", order.clientPid);
        logAndPrintMessage("Thanks Cook %d and Moto %d\n", mostEfficientCookNum, mostEfficientDeliveryPersonNum);
        messagePacket.isFinished = 1;
        sendMessagePacket(order.clientSocketFd, &messagePacket, socketMutex);

        if (mealsToDeliver != NULL) {
            free(mealsToDeliver);
        }
        close(order.clientSocketFd);
        free(meals);
        pthread_mutex_unlock(managerWorkMutex);
    }

    if (*terminationCondition == SERVER_SHUTDOWN) {
        cancelThreadPool(cookThreads, cookThreadPoolSize);
        cancelThreadPool(deliveryPersonThreads, deliveryThreadPoolSize);
    }

    joinThreadPool(cookThreads, cookThreadPoolSize);
    joinThreadPool(deliveryPersonThreads, deliveryThreadPoolSize);

    free(cookArgs);
    free(deliveryPersonArgs);

    close(mealOrderPipe[READ_END_PIPE]);
    close(mealOrderPipe[WRITE_END_PIPE]);
    close(mealCompletePipe[READ_END_PIPE]);
    close(mealCompletePipe[WRITE_END_PIPE]);

    return NULL;
}