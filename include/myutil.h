//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#ifndef MYUTIL_H
#define MYUTIL_H

#define NO_EINTR(expr) while ((expr) == -1 && errno == EINTR);
#define READ_END_PIPE 0
#define WRITE_END_PIPE 1

typedef struct {
    int portnumber;
    int cookThreadPoolSize;
    int deliveryThreadPoolSize;
    int deliverySpeed;
} ServerArguments;

typedef struct {
    int portnumber;
    int numberOfClients;
    int width;
    int height;
} ClientArguments;

typedef struct {
    int cookThreadPoolSize;
    int deliveryThreadPoolSize;
    int deliverySpeed;
    int socketFd;
    int orderPipe[2];
} ManagerArguments;

void errExit(const char* errMessage);
int parseServerArguments(int argc, char *argv[], ServerArguments *args);
int validateServerArguments(const ServerArguments *args);
int parseClientArguments(int argc, char *argv[], ClientArguments *args);
int validateClientArguments(const ClientArguments *args);

#endif //MYUTIL_H
