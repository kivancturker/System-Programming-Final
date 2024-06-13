//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#ifndef MYUTIL_H
#define MYUTIL_H

#include "queue.h"
#include "matrixops.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>

#define NO_EINTR(expr) while ((expr) == -1 && errno == EINTR);
#define READ_END_PIPE 0
#define WRITE_END_PIPE 1
#define OVEN_CAPACITY 6
#define MAX_MESSAGE_SIZE 1024

enum TerminationCondition {
    SERVER_SHUTDOWN,
    CLIENT_CANCEL,
    NO_TERMINATION
};

struct MessagePacket {
    int isFinished;
    char message[MAX_MESSAGE_SIZE];
};

struct Coord {
    int x;
    int y;
};

struct Meal {
    struct Coord coord;
    long timeTaken;
    int cookNumDealWith;
    int deliveryPersonNumDealWith;
    int customerNo;
    int clientSocketFd;
};

struct Oven {
    struct Meal meals[OVEN_CAPACITY];
    int occupiedSpots[OVEN_CAPACITY]; // 0 means available, 1 means occupied
    int mealCount;
    pthread_mutex_t mutex;
    pthread_cond_t ovenIsFull;
    int aparatusCount;
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
    pthread_mutex_t *socketMutex;
};

struct CookArguments {
    int mealOrderPipe[2];
    int mealCompletePipe[2];
    int cookNum;
    struct Oven *oven;
    pthread_mutex_t *managerWorkMutex;
    pthread_cond_t *managerWorkCond;
    pthread_cond_t *cookWorkCond;
    pthread_mutex_t *socketMutex;
};

struct DeliveryPersonArguments {
    int deliverySpeed;
    int mealToDeliverPipe[2];
    int deliveryPersonNum;
    pthread_mutex_t *socketMutex;
    pthread_cond_t *deliveryCompletedCond;
    int *deliveryCount;
    pthread_mutex_t *deliveryCountMutex;
};

struct Order {
    int numberOfClients;
    int width;
    int height;
    pid_t clientPid;
    int clientSocketFd;
    struct Meal* meals;
};

struct OrderRequest {
    int numberOfClients;
    int width;
    int height;
    pid_t pid;
    struct Meal meal;
};

struct MealToDeliver {
    struct Meal* meal[3];
    int mealCount;
    int width;
    int height;
    long timeTaken;
};

void errExit(const char* errMessage);
int parseServerArguments(int argc, char *argv[], struct ServerArguments *args);
int validateServerArguments(const struct ServerArguments *args);
int parseClientArguments(int argc, char *argv[], struct ClientArguments *args);
int validateClientArguments(const struct ClientArguments *args);
struct Coord* generateRandomCoord(int width, int height, int numberOfCoords);
int calculateDistanceFromShop(struct Coord coord, int width, int height);
void createThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), void* args);
void createCookThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), struct CookArguments* cookArgsArray);
void createDeliveryThreadPool(pthread_t* threads, int threadPoolSize, void* (*threadFunction)(void*), struct DeliveryPersonArguments* deliveryArgsArray);
void joinThreadPool(pthread_t* threads, int threadPoolSize);
void cancelThreadPool(pthread_t* threads, int threadPoolSize);
double calculateDistance(int x1, int y1, int x2, int y2);
long calculateDeliveryTime(struct MealToDeliver mealToDeliver, int width, int height, int deliverySpeed);
long calculatePseudoInverseMatrix();
int getIndexOfAvailableSpotInOven(struct Oven oven);
void placeMealInOven(struct Oven *oven, struct Meal meal);
void removeMealFromOven(struct Oven *oven, int cookNumDealWith, struct Meal *meal);
int getMostEfficientCook(struct Meal* meals, int mealCount, int cookCount);
int getMostEfficientDeliveryPerson(struct Meal* meals, int mealCount, int deliveryPersonCount);
void printCountOfMealsDeliveredByEachDeliveryPerson(struct Meal* meals, int mealCount, int deliveryPersonCount);
void printCountOfMealsPreparedByEachCook(struct Meal* meals, int mealCount, int cookCount);

#endif //MYUTIL_H
