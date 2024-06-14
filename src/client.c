//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

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
    char hostname[100];
    strncpy(hostname, args.ipAddress, 100);
    if (hostnameResolves(hostname) == 0) {
        printf("Ip Address %s could not be resolved. Either server is not running or given ip address is invalid.\n", hostname);
        return 1;
    }

    printf("PID %d\n", getpid());

    // Generate coordinates for each customer
    struct Coord* coords = generateRandomCoord(args.width, args.height, args.numberOfClients);
    if (coords == NULL) {
        errExit("generateRandomCoord");
    }
    struct Meal *meals = (struct Meal*) malloc(args.numberOfClients * sizeof(struct Meal));
    if (meals == NULL) {
        free(coords);
        errExit("malloc");
    }
    for (int i = 0; i < args.numberOfClients; i++) {
        meals[i].coord = coords[i];
        meals[i].customerNo = i + 1;
        // Default initialization
        meals[i].timeTaken = 0;
        meals[i].cookNumDealWith = -1;
        meals[i].deliveryPersonNumDealWith = -1;
        meals[i].clientSocketFd = -1;
    }

    int socketFd = connectToServer(hostname, args.portnumber);
    if (socketFd < 0) {
        free(coords);
        free(meals);
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
        if (send(socketFd, &orderRequest, sizeof(struct OrderRequest), 0) == -1) {
            close(socketFd);
            free(coords);
            free(meals);
            errExit("send in client");
        }
    }
    
    free(coords);
    free(meals);

    struct MessagePacket messagePacket = {0};

    while(1) {
        receiveMessagePacket(socketFd, &messagePacket);
        if (messagePacket.isFinished) {
            break;
        }
        printf("%s", messagePacket.message);
    }

    close(socketFd);

    return 0;
}