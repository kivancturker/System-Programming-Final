//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#ifndef MYUTIL_H
#define MYUTIL_H

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

void errExit(const char* errMessage);
int parseServerArguments(int argc, char *argv[], ServerArguments *args);
int validateServerArguments(const ServerArguments *args);
int parseClientArguments(int argc, char *argv[], ClientArguments *args);
int validateClientArguments(const ClientArguments *args);

#endif //MYUTIL_H
