//File zum Testen der OutputManagement KLasse
//Aufruf
//gcc -outputTest -Wall -Werror  outputManagement_test.c outputManagement.c

#include "outputManagement.h"

int main() {
    char text[16] = "Servus Universe!";
    printLineOnConsole(text); 
    
    printErrorMessage("Test Error Message");
    return 0;
}
