//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "myutil.h"
#include "sockconn.h"
#include "manager.h"

sig_atomic_t sigIntCount = 0;

void sigintHandler(int signal) {
    sigIntCount++;
}

int main(int argc, char *argv[]) {

    struct ServerArguments args;
    if (!parseServerArguments(argc, argv, &args)) {
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
        .managerWorkMutex = &managerWorkMutex
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
    while(sigIntCount == 0) {
        // accept connection 
        clientFd = accept(socketFd, NULL, NULL);
        if (clientFd == -1) {
            if (errno == EINTR) {
                continue;
            }
            errExit("accept");
        }
        // Log connection
        printf("New client connected\n");
        // Read the message from client
        NO_EINTR(readBytes = read(clientFd, &orderRequest, sizeof(struct OrderRequest)));
        if (readBytes == -1) {
            errExit("read");
        }
        order = (struct Order) {
            .numberOfClients = orderRequest.numberOfClients,
            .width = orderRequest.width,
            .height = orderRequest.height,
            .clientSocketFd = clientFd
        };
        // Send the connection to manager through pipe
        NO_EINTR(writtenBytes = write(orderPipe[WRITE_END_PIPE], &order, sizeof(struct Order)));
        if (writtenBytes == -1) {
            errExit("write");
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

    return 0;
}