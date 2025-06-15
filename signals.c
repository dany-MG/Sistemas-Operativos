#include "shell.h"

void ctrlC(int signum){
    signal(SIGINT, ctrlC);
    printf("\n");
    fflush(stdout);
}


