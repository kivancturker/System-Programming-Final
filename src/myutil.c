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

struct Matrix* generateMatrix(int amount) {
    struct Matrix* matrices = (struct Matrix*) malloc(amount * sizeof(struct Matrix));
    if (matrices == NULL) {
        return NULL; 
    }

    // Fill each matrix with a random number between 500 and 2000
    for (int i = 0; i < amount; i++) {
        matrices[i].number = rand() % 150001 + 250000; // rand() % 1501 gives a range of 0 to 1500
    }

    return matrices;
}

long calculatePseudoInverseMatrix(struct Matrix* matrix) {
    clock_t start = clock();

    int number = matrix->number;
    // int closestPrime = 2; // Start with the first prime number

    for (int i = 2; i <= number; i++) {
        if (isPrime(i)) {
            // closestPrime = i;
        }
    }

    clock_t end = clock();

    double durationInSeconds = (double)(end - start) / CLOCKS_PER_SEC;
    long nanoseconds = (long)(durationInSeconds * 1000000000L);

    return nanoseconds;
}

int isPrime(int num) {
    if (num <= 1) return 0; // 0 and 1 are not prime numbers
    if (num <= 3) return 1; // 2 and 3 are prime numbers

    if (num % 2 == 0 || num % 3 == 0) return 0; // Eliminate multiples of 2 and 3

    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0)
            return 0;
    }
    return 1;
}

void createThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), void* arg) {
    for (int i = 0; i < threadPoolSize; i++) {
        if (pthread_create(&threads[i], NULL, threadFunction, arg) != 0) {
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