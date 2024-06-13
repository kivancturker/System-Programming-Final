//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "myutil.h"
#include "sockconn.h"
#include "manager.h"
#include "logger.h"

sig_atomic_t sigIntCount = 0;

void sigintHandler(int signal) {
    sigIntCount++;
}

int main(int argc, char *argv[]) {

    struct ServerArguments args;
    if (!parseServerArguments(argc, argv, &args)) {
        return 1;
    }

    if (createLogFile(LOG_FILE_NAME) != 0) {
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    // Create Order pipe
    int orderPipe[2];
    if (pipe(orderPipe) == -1) {
        errExit("pipe");
    }
    enum TerminationCondition terminationCondition = NO_TERMINATION;
    pthread_cond_t managerWorkCond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t managerWorkMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t socketMutex = PTHREAD_MUTEX_INITIALIZER;

    int socketFd = establishConnection(args.portnumber);
    if (socketFd < 0) {
        errExit("establishConnection");
    }

    struct ManagerArguments managerArgs = {
        .cookThreadPoolSize = args.cookThreadPoolSize,
        .deliveryThreadPoolSize = args.deliveryThreadPoolSize,
        .deliverySpeed = args.deliverySpeed,
        .socketFd = socketFd,
        .orderPipe = {orderPipe[0], orderPipe[1]},
        .terminationCondition = &terminationCondition,
        .managerWorkCond = &managerWorkCond,
        .managerWorkMutex = &managerWorkMutex,
        .socketMutex = &socketMutex
    };
    // Create Manager Thread
    pthread_t managerThread;
    if (pthread_create(&managerThread, NULL, manager, &managerArgs) != 0) {
        errExit("pthread_create");
    }

    int clientFd = 0;
    struct OrderRequest orderRequest;
    struct Order order;
    int readBytes = 0;
    int writtenBytes = 0;
    int connectedClientCount = 0;
    int numberOfClients = 0;
    int i = 0;
    struct Meal *meals;
    while(sigIntCount == 0) {
        logAndPrintMessage("PideShop active waiting for connection ...\n");
        // accept connection 
        clientFd = accept(socketFd, NULL, NULL);
        if (clientFd == -1) {
            if (errno == EINTR) {
                continue;
            }
            errExit("accept");
        }
        // Log connection
        connectedClientCount++;
        // Read the message from client
        do {
            NO_EINTR(readBytes = read(clientFd, &orderRequest, sizeof(struct OrderRequest)));
            if (readBytes == -1) {
                errExit("read socket in server");
            }
            numberOfClients = orderRequest.numberOfClients;
            if (i == 0) {
                meals = (struct Meal*) malloc(numberOfClients * sizeof(struct Meal));
            }
            meals[i] = orderRequest.meal;
            i++;
        } while(i < numberOfClients);
        i = 0;
        numberOfClients = 0;

        order = (struct Order) {
                .numberOfClients = orderRequest.numberOfClients,
                .width = orderRequest.width,
                .height = orderRequest.height,
                .clientSocketFd = clientFd,
                .meals = meals,
                .clientPid = orderRequest.pid
        };
        NO_EINTR(writtenBytes = write(orderPipe[WRITE_END_PIPE], &order, sizeof(struct Order)));
        if (writtenBytes == -1) {
            errExit("write to socket in server");
        }

        pthread_mutex_lock(&managerWorkMutex);
        pthread_cond_signal(&managerWorkCond);
        pthread_mutex_unlock(&managerWorkMutex);
    }
    if (sigIntCount != 0) {
            printf("Server is shutting down\n");
            pthread_mutex_lock(&managerWorkMutex);
            terminationCondition = SERVER_SHUTDOWN;
            pthread_cond_signal(&managerWorkCond);
            pthread_mutex_unlock(&managerWorkMutex);
    
    }

    // Join Manager Thread
    if (pthread_join(managerThread, NULL) != 0) {
        errExit("pthread_join");
    }

    close(socketFd);
    closeLogFile();

    return 0;
}