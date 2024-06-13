#ifndef SOCKCONN_H
#define SOCKCONN_H

#include "myutil.h"

#include <pthread.h>

#define MAXHOSTNAME 256
#define BACKLOG_LIMIT 5

int establishConnection(int portnum);
int connectToServer(const char *hostname, int portnum);
int sendMessagePacket(int socketFd, const struct MessagePacket *packet, pthread_mutex_t *socketMutex);
int receiveMessagePacket(int socketFd, struct MessagePacket *packet);

#endif // SOCKCONN_H