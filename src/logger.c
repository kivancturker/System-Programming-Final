#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

// File pointer for the log file
static FILE* logFile = NULL;

// Mutex for thread-safe logging
static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

// Create or truncate the log file
int createLogFile(const char* logFileName) {
    logFile = fopen(logFileName, "w");
    if (logFile == NULL) {
        perror("Error opening log file");
        return -1;
    }
    return 0;
}

// Log a formatted message to the log file
void logMessage(const char* format, ...) {
    pthread_mutex_lock(&logMutex);

    if (logFile == NULL) {
        logFile = fopen(LOG_FILE_NAME, "a");
        if (logFile == NULL) {
            perror("Error opening log file");
            pthread_mutex_unlock(&logMutex);
            return;
        }
    }

    va_list args;
    va_start(args, format);
    vfprintf(logFile, format, args);
    fprintf(logFile, "\n");
    va_end(args);

    fflush(logFile);
    pthread_mutex_unlock(&logMutex);
}

void logAndPrintMessage(const char* format, ...) {
    pthread_mutex_lock(&logMutex);

    if (logFile == NULL) {
        logFile = fopen(LOG_FILE_NAME, "a");
        if (logFile == NULL) {
            perror("Error opening log file");
            pthread_mutex_unlock(&logMutex);
            return;
        }
    }

    va_list args1, args2;
    va_start(args1, format);
    va_start(args2, format);
    vfprintf(logFile, format, args1);
    vprintf(format, args2);
    va_end(args1);
    va_end(args2);

    fflush(logFile);
    pthread_mutex_unlock(&logMutex);
}

void closeLogFile() {
    pthread_mutex_lock(&logMutex);
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
    pthread_mutex_unlock(&logMutex);
}