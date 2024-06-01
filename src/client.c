//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "myutil.h"
#include "sockconn.h"

sig_atomic_t sigIntrCount = 0;

void sigintHandler(int signal) {
    sigIntrCount++;
}

int main(int argc, char *argv[]) {

    ClientArguments args;
    if (!parseClientArguments(argc, argv, &args)) {
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    char *defaultHostname = "localhost";

    int socketFd = connectToServer(defaultHostname, args.portnumber);

    close(socketFd);

    return 0;
}