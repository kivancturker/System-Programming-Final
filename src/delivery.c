#include "delivery.h"
#include "myutil.h"
#include "sockconn.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

void* deliveryPerson(void* arg) {
    struct DeliveryPersonArguments* args = (struct DeliveryPersonArguments*) arg;
    int mealToDeliverPipe[2] = {args->mealToDeliverPipe[0], args->mealToDeliverPipe[1]};
    int deliverySpeed = args->deliverySpeed;
    int deliveryPersonNum = args->deliveryPersonNum;
    pthread_mutex_t *socketMutex = args->socketMutex;
    pthread_mutex_t *deliveryCountMutex = args->deliveryCountMutex;
    int *deliveryCount = args->deliveryCount;
    pthread_cond_t *deliveryCompletedCond = args->deliveryCompletedCond;

    // In a loop read from the pipe and print the result after that close the client socket
    struct MealToDeliver* mealToDeliver = NULL;
    int readBytes = 0;
    struct MessagePacket packet = {0};
    int clientSocketFd = -1;
    while (1) {
        NO_EINTR(readBytes = read(mealToDeliverPipe[READ_END_PIPE], &mealToDeliver, sizeof(struct MealToDeliver*)));
        if (readBytes == -1) {
            errExit("read from mealToDeliverPipe");
        }

        if (mealToDeliver == NULL) {
            fprintf(stderr, "Received null pointer for MealToDeliver\n");
            continue;
        }

        for (int i = 0; i < mealToDeliver->mealCount; i++) {
            mealToDeliver->meal[i].deliveryPersonNumDealWith = deliveryPersonNum;
        }
        clientSocketFd = mealToDeliver->meal[0].clientSocketFd;

        snprintf(packet.message, MAX_MESSAGE_SIZE, "DP %d STARTED delivering to customer %d\n", 
                    deliveryPersonNum, mealToDeliver->meal[0].customerNo);
        sendMessagePacket(clientSocketFd, &packet, socketMutex);
        logMessage(packet.message);

        long timeTakenInMilliseconds = calculateDeliveryTime(*mealToDeliver, mealToDeliver->width, mealToDeliver->height, deliverySpeed);
        timeTakenInMilliseconds /= 10; // Speed up the delivery process by 10 times
        // Convert milliseconds to timespec
        struct timespec req;
        req.tv_sec = timeTakenInMilliseconds / 1000; // Convert to seconds
        req.tv_nsec = (timeTakenInMilliseconds % 1000) * 1000000L; // Convert remaining milliseconds to nanoseconds

        // Sleep for the calculated time
        nanosleep(&req, NULL);

        snprintf(packet.message, MAX_MESSAGE_SIZE, "DP %d FINISHED delivering to customer %d\n", 
                    deliveryPersonNum, mealToDeliver->meal[0].customerNo);
        sendMessagePacket(clientSocketFd, &packet, socketMutex);
        logMessage(packet.message);

        free(mealToDeliver);
        mealToDeliver = NULL;
        
        pthread_mutex_lock(deliveryCountMutex);
        (*deliveryCount)--;
        pthread_mutex_unlock(deliveryCountMutex);
        pthread_cond_signal(deliveryCompletedCond);
    }

    return NULL;
}