//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#ifndef MYUTIL_H
#define MYUTIL_H

#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define NO_EINTR(expr) while ((expr) == -1 && errno == EINTR);
#define READ_END_PIPE 0
#define WRITE_END_PIPE 1

enum TerminationCondition {
    SERVER_SHUTDOWN,
    CLIENT_CANCEL,
    NO_TERMINATION
};

struct ServerArguments {
    int portnumber;
    int cookThreadPoolSize;
    int deliveryThreadPoolSize;
    int deliverySpeed;
};

struct ClientArguments {
    int portnumber;
    int numberOfClients;
    int width;
    int height;
};

struct ManagerArguments {
    int cookThreadPoolSize;
    int deliveryThreadPoolSize;
    int deliverySpeed;
    int socketFd;
    int orderPipe[2];
    enum TerminationCondition *terminationCondition;
    pthread_cond_t *managerWorkCond;
    pthread_mutex_t *managerWorkMutex;
};

struct Order {
    int numberOfClients;
    int width;
    int height;
    int clientSocketFd;
};

struct OrderRequest {
    int numberOfClients;
    int width;
    int height;
};

struct Coord {
    int x;
    int y;
};

void errExit(const char* errMessage);
int parseServerArguments(int argc, char *argv[], struct ServerArguments *args);
int validateServerArguments(const struct ServerArguments *args);
int parseClientArguments(int argc, char *argv[], struct ClientArguments *args);
int validateClientArguments(const struct ClientArguments *args);
struct Coord* generateRandomCoord(int width, int height, int numberOfCoords);
int calculateDistanceFromShop(struct Coord coord, int width, int height);

#endif //MYUTIL_H
