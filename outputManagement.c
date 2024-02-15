#include "outputManagement.h"
#include <stdio.h>

void printLineOnConsole(char* msg){
    printf("%s\n", msg);
}

void printErrorMessage(char* msg){
    fprintf(stderr, "Error: %s\n", msg);
}
