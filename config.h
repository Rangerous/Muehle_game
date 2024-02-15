#pragma once
#define MAX_LINELENGTH 60

typedef struct config_struct {
    char hostname[MAX_LINELENGTH];
    char port[MAX_LINELENGTH];
    char gameKindName[MAX_LINELENGTH];
    } confStruct;
    
confStruct config;

int readConfig(char *dateiname);
