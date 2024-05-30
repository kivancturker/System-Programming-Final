//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "myutil.h"

sig_atomic_t sigIntrCount = 0;

void sigintHandler(int signal) {
    sigIntrCount++;
}

int main(int argc, char *argv[]) {

    ServerArguments args;
    if (!parseServerArguments(argc, argv, &args)) {
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    return 0;
}