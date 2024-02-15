//File zum Testen der playerHandling KLasse
//Aufruf
//make playerTest

#include "playerHandling.h"
#include <stdio.h>


int main() {
    
    Player playerOne = {
        "maxTest", 42
    };
    
    printf("Test typedef struct Player: Name: %s, ID: %i\n", playerOne.name, playerOne.ID);
    return 0;
}
