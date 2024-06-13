//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include "myutil.h"

void errExit(const char* errMessage) {
    perror(errMessage);
    exit(EXIT_FAILURE);
}

int parseServerArguments(int argc, char *argv[], struct ServerArguments *args) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <portnumber> <CookthreadPoolSize> <DeliveryPoolSize> <deliverySpeed>\n", argv[0]);
        return 0;
    }

    args->portnumber = atoi(argv[1]);
    args->cookThreadPoolSize = atoi(argv[2]);
    args->deliveryThreadPoolSize = atoi(argv[3]);
    args->deliverySpeed = atoi(argv[4]);

    return validateServerArguments(args);
}

int validateServerArguments(const struct ServerArguments *args) {
    if (args->portnumber <= 0) {
        fprintf(stderr, "Error: portnumber must be a positive integer.\n");
        return 0;
    }
    if (args->cookThreadPoolSize <= 0) {
        fprintf(stderr, "Error: CookthreadPoolSize must be a positive integer.\n");
        return 0;
    }
    if (args->deliveryThreadPoolSize <= 0) {
        fprintf(stderr, "Error: DeliveryPoolSize must be a positive integer.\n");
        return 0;
    }
    if (args->deliverySpeed <= 0) {
        fprintf(stderr, "Error: deliverySpeed must be a positive integer.\n");
        return 0;
    }
    return 1;
}

int parseClientArguments(int argc, char *argv[], struct ClientArguments *args) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <portnumber> <numberOfClients> <townWidth> <townHeight>\n", argv[0]);
        return 0;
    }

    args->portnumber = atoi(argv[1]);
    args->numberOfClients = atoi(argv[2]);
    args->width = atoi(argv[3]);
    args->height = atoi(argv[4]);

    return validateClientArguments(args);
}

int validateClientArguments(const struct ClientArguments *args) {
    if (args->portnumber <= 0) {
        fprintf(stderr, "Error: portnumber must be a positive integer.\n");
        return 0;
    }
    if (args->numberOfClients <= 0) {
        fprintf(stderr, "Error: numberOfClients must be a positive integer.\n");
        return 0;
    }
    if (args->width <= 0) {
        fprintf(stderr, "Error: townWidth must be a positive integer.\n");
        return 0;
    }
    if (args->height <= 0) {
        fprintf(stderr, "Error: townHeight must be a positive integer.\n");
        return 0;
    }
    return 1;
}

struct Coord* generateRandomCoord(int width, int height, int numberOfCoords) {
    struct Coord* coords = (struct Coord*) malloc(numberOfCoords * sizeof(struct Coord));
    if (coords == NULL) {
        return NULL;
    }

    for (int i = 0; i < numberOfCoords; i++) {
        coords[i].x = rand() % width;
        coords[i].y = rand() % height;
    }

    return coords;
}

int calculateDistanceFromShop(struct Coord coord, int width, int height) {
    int shopX = width / 2;
    int shopY = height / 2;

    return (int) sqrt(pow(coord.x - shopX, 2) + pow(coord.y - shopY, 2));
}

long calculatePseudoInverseMatrix() {
    clock_t start = clock();

    double A[COLS][ROWS];
    generatePseudoInverseMatrix(A);

    clock_t end = clock();

    double durationInSeconds = (double)(end - start) / CLOCKS_PER_SEC;
    long nanoseconds = (long)(durationInSeconds * 1000000000L);

    return nanoseconds;
}

void createThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), void* args) {
    for (int i = 0; i < threadPoolSize; i++) {
        if (pthread_create(&threads[i], NULL, threadFunction, &args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}

void createCookThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), struct CookArguments* cookArgsArray) {
    for (int i = 0; i < threadPoolSize; i++) {
        cookArgsArray[i].cookNum = i; // Assign thread number
        if (pthread_create(&threads[i], NULL, threadFunction, &cookArgsArray[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}

void createDeliveryThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), struct DeliveryPersonArguments* deliveryArgsArray) {
    for (int i = 0; i < threadPoolSize; i++) {
        deliveryArgsArray[i].deliveryPersonNum = i; // Assign thread number
        if (pthread_create(&threads[i], NULL, threadFunction, &deliveryArgsArray[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
}


void joinThreadPool(pthread_t* threads, int threadPoolSize) {
    for (int i = 0; i < threadPoolSize; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
}

void cancelThreadPool(pthread_t* threads, int threadPoolSize) {
    for (int i = 0; i < threadPoolSize; i++) {
        if (pthread_cancel(threads[i]) != 0) {
            perror("pthread_cancel");
            exit(EXIT_FAILURE);
        }
    }
}

// Helper function to calculate the Euclidean distance between two points
double calculateDistance(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Calculate the delivery time based on coordinates and delivery speed
long calculateDeliveryTime(struct MealToDeliver mealToDeliver, int width, int height, int deliverySpeed) {
    int centerX = width / 2;
    int centerY = height / 2;
    double totalDistance = 0.0;

    struct Coord prevCoord = {centerX, centerY};
    for (int i = 0; i < mealToDeliver.mealCount; i++) {
        struct Coord coord = mealToDeliver.meal[i].coord;
        // Calculate distance from shop to each delivery point
        totalDistance += calculateDistance(prevCoord.x, prevCoord.y, coord.x, coord.y);
        prevCoord = coord;
    }

    // Account for the return trip distance
    totalDistance += calculateDistance(prevCoord.x, prevCoord.y, centerX, centerY);

    // Calculate time in seconds, then convert to milliseconds
    if (deliverySpeed == 0) {
        deliverySpeed = 1;
    }
    double timeInSeconds = totalDistance / deliverySpeed;
    long timeInMilliseconds = (long)(timeInSeconds * 1000);

    return timeInMilliseconds;
}

int getIndexOfAvailableSpotInOven(struct Oven oven) {
    for (int i = 0; i < OVEN_CAPACITY; i++) {
        if (oven.occupiedSpots[i] == 0) {
            return i;
        }
    }
    return -1; // No available spot found
}

void placeMealInOven(struct Oven *oven, struct Meal meal) {
    pthread_mutex_lock(&oven->mutex);

    // Wait until there is an available spot
    while (oven->mealCount >= OVEN_CAPACITY) {
        pthread_cond_wait(&oven->ovenIsFull, &oven->mutex);
    }

    // Find the first available spot
    int index = getIndexOfAvailableSpotInOven(*oven);

    if (index != -1) {
        // Place the meal in the oven
        oven->meals[index] = meal;
        oven->occupiedSpots[index] = 1; // Mark the spot as occupied
        oven->mealCount++;
        oven->aparatusCount--;
    }

    pthread_mutex_unlock(&oven->mutex);
}

void removeMealFromOven(struct Oven *oven, int cookNumDealWith, struct Meal *meal) {
    pthread_mutex_lock(&oven->mutex);

    // Find the meal associated with the requesting cook
    int index = -1;
    for (int i = 0; i < OVEN_CAPACITY; i++) {
        if (oven->occupiedSpots[i] == 1 && oven->meals[i].cookNumDealWith == cookNumDealWith) {
            index = i;
            break;
        }
    }

    // Remove the meal from the oven
    if (index != -1) {
        *meal = oven->meals[index]; // Copy the meal to the provided pointer
        oven->occupiedSpots[index] = 0; // Mark the spot as available
        oven->mealCount--;
        oven->aparatusCount++;

        // Signal that a spot is now available
        pthread_cond_broadcast(&oven->ovenIsFull);
    } else {
        fprintf(stderr, "Error: No meal found for the requesting cook.\n");
    }

    pthread_mutex_unlock(&oven->mutex);
}