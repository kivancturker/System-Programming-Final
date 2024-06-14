#include "sockconn.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int establishConnection(int portnum) {
    char myname[MAXHOSTNAME+1];
    int socketFd;
    struct sockaddr_in sa;
    struct hostent *hp;
    memset(&sa, 0, sizeof(struct sockaddr_in));
    gethostname(myname, MAXHOSTNAME);
    hp = gethostbyname(myname);
    if (hp == NULL) {
        return -1;
    }

    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(portnum);

    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (bind(socketFd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
        close(socketFd);
        return -1;
    }

    if (listen(socketFd, BACKLOG_LIMIT) < 0) {
        close(socketFd);
        return -1;
    }

    return socketFd;
}

int connectToServer(const char *hostname, int portnum) {
    int socketFd;
    struct sockaddr_in sa;
    struct hostent *hp;
    memset(&sa, 0, sizeof(struct sockaddr_in));
    hp = gethostbyname(hostname);
    if (hp == NULL) {
        return -1;
    }

    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(portnum);
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);

    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (connect(socketFd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
        close(socketFd);
        return -1;
    }

    return socketFd;
}

int hostnameResolves(const char* hostname) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Datagram socket

    int status = getaddrinfo(hostname, NULL, &hints, &res);
    if (status != 0) {
        return 0; // Failed to resolve
    }

    freeaddrinfo(res);
    return 1; // Successfully resolved
}

int sendMessagePacket(int socketFd, const struct MessagePacket *packet, pthread_mutex_t *socketMutex) {
    pthread_mutex_lock(socketMutex);
    ssize_t bytesSent = 0;
    ssize_t totalSize = sizeof(*packet);

    while (bytesSent < totalSize) {
        ssize_t result = send(socketFd, (const char*)packet + bytesSent, totalSize - bytesSent, 0);
        if (result == -1) {
            perror("send");
            pthread_mutex_unlock(socketMutex);
            return -1;
        }
        bytesSent += result;
    }

    pthread_mutex_unlock(socketMutex);
    return 0;
}

int receiveMessagePacket(int socketFd, struct MessagePacket *packet) {
    ssize_t bytesRead = 0;
    ssize_t totalSize = sizeof(*packet);

    while (bytesRead < totalSize) {
        ssize_t result = recv(socketFd, (char*)packet + bytesRead, totalSize - bytesRead, 0);
        if (result == -1) {
            perror("recv");
            return -1;
        }
        bytesRead += result;
    }

    return 0;
}
