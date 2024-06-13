#include "cook.h"
#include "myutil.h"
#include "sockconn.h"
#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

void* cook(void* arg) {
    struct CookArguments* args = (struct CookArguments*) arg;
    int mealOrderPipe[2] = {args->mealOrderPipe[0], args->mealOrderPipe[1]};
    int mealCompletePipe[2] = {args->mealCompletePipe[0], args->mealCompletePipe[1]};
    pthread_mutex_t* managerWorkMutex = args->managerWorkMutex;
    pthread_cond_t* managerWorkCond = args->managerWorkCond;
    pthread_cond_t* cookWorkCond = args->cookWorkCond;
    struct Oven *oven = args->oven;
    int cookNum = args->cookNum;
    pthread_mutex_t *socketMutex = args->socketMutex;

    // In a loop read from the pipe and print the result after that close the client socket
    struct Meal meal;
    int readBytes = 0;
    int writeBytes = 0;
    long timeTaken = 0;
    struct timespec ts;
    int clientSocketFd = -1;
    struct MessagePacket packet = {0};
    while (1) {
        NO_EINTR(readBytes = read(mealOrderPipe[READ_END_PIPE], &meal, sizeof(struct Meal)));
        if (readBytes == -1) {
            errExit("read from mealOrderPipe");
        }
        clientSocketFd = meal.clientSocketFd;
        snprintf(packet.message, MAX_MESSAGE_SIZE, "Cook %d STARTED preparing meal for customer %d\n", 
                    cookNum, meal.customerNo);
        sendMessagePacket(clientSocketFd, &packet, socketMutex);
        logMessage(packet.message);

        // Meal preparation step
        meal.timeTaken = calculatePseudoInverseMatrix();
        meal.cookNumDealWith = cookNum;

        // Place it in the oven
        placeMealInOven(oven, meal);

        // Wait for half the time
        timeTaken = meal.timeTaken / 2;
        ts.tv_sec = timeTaken / 1000000000L;
        ts.tv_nsec = timeTaken % 1000000000L;
        nanosleep(&ts, NULL);

        // Take it out of the oven
        removeMealFromOven(oven, cookNum, &meal);
        snprintf(packet.message, MAX_MESSAGE_SIZE, "Cook %d FINISHED preparing meal for customer %d\n", 
                    cookNum, meal.customerNo);
        sendMessagePacket(clientSocketFd, &packet, socketMutex);
        logMessage(packet.message);
        
        NO_EINTR(writeBytes = write(mealCompletePipe[WRITE_END_PIPE], &meal, sizeof(struct Meal)));
        if (writeBytes == -1) {
            errExit("write to mealCompletePipe");
        }
    }

    return NULL;
}