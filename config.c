#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

int readConfig(char *dateiname){
    ssize_t nRet;
    size_t len = 0;
    char *line = NULL;
    
    FILE *datei;
    datei = fopen(dateiname, "r");
    if(datei == NULL) {
        printf("Konnte Config-Datei nicht öffnen!\n");
        return EXIT_FAILURE;
    }
    while( (nRet=getline(&line, &len, datei)) != -1){
        const int line_length = strlen(line);
        char befEq[line_length];
        memset(befEq, '\0', 1);
        char befEq1[line_length];
        memset(befEq1, '\0', 1);
        char aftEq[line_length];
        memset(aftEq, '\0', 1);
        sscanf(line, " %[^=]= %s ", befEq, aftEq);
        sscanf(befEq, " %s ", befEq1);
        if(strncmp(befEq1, "hostname", 8) == 0){
            strcpy(config.hostname, aftEq);
        }
        if(strncmp(befEq1, "port", 4) == 0){
            strcpy(config.port, aftEq);
        }
        if(strncmp(befEq1, "gameKindName", 12) == 0){
            strcpy(config.gameKindName, aftEq);
        }
    }
    free(line);
    fclose(datei);
    return 0;
}
