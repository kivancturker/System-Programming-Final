#include "sockconn.h"

#include <sys/socket.h>
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