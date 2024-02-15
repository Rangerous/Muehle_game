#pragma once
#include "playerHandling.h"

int createMemory ();


typedef struct {
    char* gameID;
    char gamename [64];
    Player players[2]; // mehrere Spieler sollten implementiert sein
    int nplayers;
    pid_t connectorid;
    pid_t thinkerid;
    bool shouldThink;
    char playingField [3][8];
    int desiredPlayerId;
    int clientPlayerId;
    int maxMoveTime;
    int capturedPieceCount;
    bool shouldCapture;
    int playerPieceCount;
} shmContent_t;

shmContent_t *shmdata;

//Pipe
int fd[2];
