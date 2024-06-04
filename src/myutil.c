//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <stdlib.h>

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