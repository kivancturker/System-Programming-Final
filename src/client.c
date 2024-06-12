//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "myutil.h"
#include "sockconn.h"

sig_atomic_t sigIntrCount = 0;

void sigintHandler(int signal) {
    sigIntrCount++;
}

int main(int argc, char *argv[]) {

    struct ClientArguments args;
    if (!parseClientArguments(argc, argv, &args)) {
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    char *defaultHostname = "localhost";

    printf("PID %d\n", getpid());

    // Generate coordinates for each customer
    struct Coord* coords = generateRandomCoord(args.width, args.height, args.numberOfClients);
    struct Matrix* matrices = generateMatrix(args.numberOfClients);
    struct Meal *meals = (struct Meal*) malloc(args.numberOfClients * sizeof(struct Meal));
    for (int i = 0; i < args.numberOfClients; i++) {
        meals[i].matrix = matrices[i];
        meals[i].coord = coords[i];
        meals[i].customerNo = i + 1;
    }

    int socketFd = connectToServer(defaultHostname, args.portnumber);
    if (socketFd < 0) {
        errExit("connectToServer");
    }

    struct OrderRequest orderRequest = {0};

    for (int i = 0; i < args.numberOfClients; i++) {
        orderRequest.numberOfClients = args.numberOfClients;
        orderRequest.width = args.width;
        orderRequest.height = args.height;
        orderRequest.meal = meals[i];
        orderRequest.pid = getpid();

        // Send the order request including one meal
        if (write(socketFd, &orderRequest, sizeof(orderRequest)) == -1) {
            errExit("write");
        }
    }

    free(coords);
    free(matrices);
    free(meals);

    close(socketFd);

    return 0;
}