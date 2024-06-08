#include "cook.h"
#include "myutil.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* cook(void* arg) {
    struct CookArguments* args = (struct CookArguments*) arg;
    int mealOrderPipe[2] = {args->mealOrderPipe[0], args->mealOrderPipe[1]};
    int mealCompletePipe[2] = {args->mealCompletePipe[0], args->mealCompletePipe[1]};
    pthread_mutex_t* managerWorkMutex = args->managerWorkMutex;
    pthread_cond_t* managerWorkCond = args->managerWorkCond;
    pthread_cond_t* cookWorkCond = args->cookWorkCond;
    pthread_mutex_t *mealOrderPipeMutex = args->mealOrderPipeMutex;
    pthread_mutex_t *mealCompletePipeMutex = args->mealCompletePipeMutex;

    // In a loop read from the pipe and print the result after that close the client socket
    struct Meal meal;
    int readBytes = 0;
    int writeBytes = 0;
    long timeTaken = 0;
    while (1) {
        NO_EINTR(readBytes = read(mealOrderPipe[READ_END_PIPE], &meal, sizeof(struct Meal)));
        if (readBytes == -1) {
            errExit("read");
        }

        meal.timeTaken = calculatePseudoInverseMatrix(&meal.matrix);
        
        NO_EINTR(writeBytes = write(mealCompletePipe[WRITE_END_PIPE], &meal, sizeof(struct Meal)));
        if (writeBytes == -1) {
            errExit("write");
        }
    }

    return NULL;
}