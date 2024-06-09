#include "delivery.h"
#include "myutil.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>

void* deliveryPerson(void* arg) {
    struct DeliveryPersonArguments* args = (struct DeliveryPersonArguments*) arg;
    int mealToDeliverPipe[2] = {args->mealToDeliverPipe[0], args->mealToDeliverPipe[1]};
    int deliverySpeed = args->deliverySpeed;

    // In a loop read from the pipe and print the result after that close the client socket
    struct MealToDeliver mealToDeliver;
    int readBytes = 0;
    while (1) {
        NO_EINTR(readBytes = read(mealToDeliverPipe[READ_END_PIPE], &mealToDeliver, sizeof(struct MealToDeliver)));
        if (readBytes == -1) {
            errExit("read");
        }

        long timeTakenInMilliseconds = calculateDeliveryTime(mealToDeliver, mealToDeliver.width, mealToDeliver.height, deliverySpeed);
        
        // Convert milliseconds to timespec
        struct timespec req;
        req.tv_sec = timeTakenInMilliseconds / 1000; // Convert to seconds
        req.tv_nsec = (timeTakenInMilliseconds % 1000) * 1000000L; // Convert remaining milliseconds to nanoseconds

        // Sleep for the calculated time
        nanosleep(&req, NULL);

        printf("Delivering mealCount %d and time is: %ld\n", mealToDeliver.mealCount, timeTakenInMilliseconds);
    }

    return NULL;
}