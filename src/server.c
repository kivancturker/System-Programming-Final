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

    ServerArguments args;
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

    int socketFd = establishConnection(args.portnumber);
    if (socketFd < 0) {
        errExit("establishConnection");
    }

    ManagerArguments managerArgs = {
        .cookThreadPoolSize = args.cookThreadPoolSize,
        .deliveryThreadPoolSize = args.deliveryThreadPoolSize,
        .deliverySpeed = args.deliverySpeed,
        .socketFd = socketFd,
        .orderPipe = {orderPipe[0], orderPipe[1]}
    };
    // Create Manager Thread
    pthread_t managerThread;
    if (pthread_create(&managerThread, NULL, manager, &managerArgs) != 0) {
        errExit("pthread_create");
    }

    int clientFd;
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
        // Send the connection to manager through pipe
        close(clientFd);
    }
    if (sigIntCount != 0) {
            printf("Server is shutting down\n");
    
    }

    // Join Manager Thread
    if (pthread_join(managerThread, NULL) != 0) {
        errExit("pthread_join");
    }

    close(socketFd);

    return 0;
}