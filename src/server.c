//
// Created by Kıvanç TÜRKER on 29.05.2024.
//

#include <stdio.h>

#include "myutil.h"

int main(int argc, char *argv[]) {

    ServerArguments args;
    if (!parseServerArguments(argc, argv, &args)) {
        return 1;
    }

    return 0;
}