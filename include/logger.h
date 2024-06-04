#ifndef LOGGER_H
#define LOGGER_H

#define LOG_FILE_NAME "log.txt"

int createLogFile(const char* logFileName);
void logMessage(const char* format, ...);
void logAndPrintMessage(const char* format, ...);
void closeLogFile();

#endif // LOGGER_H