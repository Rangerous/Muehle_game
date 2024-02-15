#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>
#include "think.h"
#include "memoryManagement.h"
#include "outputManagement.h"

bool calculateMove( char *returnMove);
bool calculateMoveWhenMillPossible(char *returnMove, bool toSet);
bool calculateRandomMove( char *returnMove, bool toSet);
void getNameOfPosition(bool toSet, char[2] , int ring, int positionInRing, char *returnMove);
bool calculateCapture(char *returnMove);
bool calculateRandomCapture(char *returnMove);
bool isPieceInMuele(int ring, int positionInRing, char sing, int positionID[2]);
bool hasFreeNeighbor(int positionID[2]);

void think (int sig){
    if (shmdata->shouldThink == false){
        return;
    }

    printf("Start Thinking \n");
        
    //Neuen Zug berechnen
    char move[6] = "\0";
    
    
    if(shmdata->shouldCapture == true) {
        calculateCapture(move);
    } else {
        calculateMove(move);
    }
        
    //Spielstand ausgeben
    move[5] = '\0';
    if (debugAusgabe) printf("Send to Pipe: %s \n", move);
    int pipeLenght = 6;
    if (write(fd[1], move, pipeLenght) != pipeLenght) {
        perror ("write");
        exit (EXIT_FAILURE);
    }
    
    shmdata->shouldThink = false; 
}

bool calculateMove(char *returnMove) {
    bool toSet = false;
    for(int i = 8; i >= 0; i--){
        if(!shmdata->players[shmdata->clientPlayerId].piecelist[i].isSet 
        && !shmdata->players[shmdata->clientPlayerId].piecelist[i].isCaptured) {
            if (debugAusgabe) printf("Calculate Set\n");
            toSet = true;
            break;
        }
    }
    
    if (calculateMoveWhenMillPossible(returnMove, toSet)) {
        return true;
    } else {
        return calculateRandomMove(returnMove, toSet);
    }
}

bool calculateCapture(char *returnMove) {
    return calculateRandomCapture(returnMove);
}

bool calculateRandomCapture(char *returnMove) {
    if (debugAusgabe) printf("Calculate Random Capture\n");
    bool randomFound = false;
    srand(time(NULL));
    
    int ring;
    int positionInRing;
    
    bool toSet = false;
    bool allInMill = true;
    char oldPos[2] = "\0";
    int oldPosID[2];
        oldPosID[0] = -1;
        oldPosID[1] = -1;
        
    for(int i = 8; i >= 0; i--){
        if(shmdata->players[shmdata->clientPlayerId].piecelist[i].isSet 
            && !isPieceInMuele(shmdata->players[(shmdata->clientPlayerId +1 ) % 2].piecelist[i].positionID[0],shmdata->players[(shmdata->clientPlayerId + 1 ) % 2].piecelist[i].positionID[0],shmdata->players[shmdata->clientPlayerId].sign, oldPosID)) {
            allInMill = false;
            break;
        }
    }
    
    while(!randomFound) {
        ring = rand() % 3;
        positionInRing = rand() % 8;
        
        char opponentSign = shmdata->players[(shmdata->clientPlayerId + 1) % 2].sign;
        if(shmdata->playingField[ring][positionInRing] == opponentSign
            && (allInMill || !isPieceInMuele(ring, positionInRing, opponentSign, oldPosID))) {
            if (debugAusgabe) printf("Capture: %i %i\n", ring, positionInRing);
            shmdata->shouldCapture = false;
            toSet = true;
            getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
            return true;
        } 
    } 
    
    return false;
}

bool calculateMoveWhenMillPossible(char *returnMove, bool toSet) {
    
    char oldPos[2] = "\0";
    int oldPosID[2];
        oldPosID[0] = -1;
        oldPosID[1] = -1;
    int ring = 0;
    int positionInRing = 0;
        
    if (shmdata->players[shmdata->clientPlayerId].capturedPieces >= 6 || toSet){
        for( ring = 0; ring < 3; ring++) {
            for ( positionInRing = 0; positionInRing < 8; positionInRing++) {
                if (shmdata->playingField[ring][positionInRing] == '0') {
                    if(toSet
                        && (isPieceInMuele(ring, positionInRing, shmdata->players[0].sign, oldPosID)
                            || isPieceInMuele(ring, positionInRing, shmdata->players[1].sign, oldPosID))) {
                        if (debugAusgabe) printf("Have mill found mill with Set\n");
                        getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
                        return true;
                    } else {
                        for (int ownPiece = 0; ownPiece < 9; ownPiece++) {
                            if(shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].isSet
                                && (isPieceInMuele(ring, positionInRing, shmdata->players[0].sign, shmdata->players[0].piecelist[ownPiece].positionID)
                                    || isPieceInMuele(ring, positionInRing, shmdata->players[1].sign, shmdata->players[1].piecelist[ownPiece].positionID))) {
                                oldPos[0] = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].position[0];
                                oldPos[1] = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].position[1];
                                if (debugAusgabe) printf("Have mill found mill with Jump\n");
                                
                                getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
                                return true;    
                            }
                        }
                    }
                }
            }
        }
    } else {
        for ( int ownPiece = 0; ownPiece < 9; ownPiece++) {
            if(shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].isSet
            && hasFreeNeighbor(shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID)) {
                
                oldPos[0] = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].position[0];
                oldPos[1] = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].position[1];
                
                ring = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID[0];
                positionInRing = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID[1];
                
                int possibleDirections = 0;
                if (positionInRing == 1 || positionInRing == 3 || positionInRing == 5 || positionInRing == 7 ) {
                    possibleDirections = 4;
                } else {
                    possibleDirections = 2;
                }
                
                for ( int direction = 0; direction < possibleDirections; direction++) {
                    ring = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID[0];
                    positionInRing = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID[1];
                    
                    if(direction == 0) { //Rechts
                        if (positionInRing == 7) {
                            positionInRing = 0;
                        } else {
                            positionInRing = positionInRing + 1;
                        }
                    } else if (direction == 1) { //Links
                        if (positionInRing == 0) {
                            positionInRing = 7;
                        } else {
                            positionInRing = positionInRing - 1;
                        }
                    } else if (direction == 2){ //Wenn möglich hoch
                        if (ring != 0) {//nicht Aussen
                            ring = ring -1;
                        } else {
                            ring = ring +1;
                        }
                    } else { //Wenn möglich runter
                        if (ring != 2) {//nicht Innen
                            ring = ring +1;
                        } else {
                            ring = ring -1;
                        }
                    }
                
                    if (shmdata->playingField[ring][positionInRing] == '0'
                        && (isPieceInMuele(ring, positionInRing, shmdata->players[0].sign, shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID)
                            || isPieceInMuele(ring, positionInRing, shmdata->players[1].sign, shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID))){
                                if (debugAusgabe) printf("Have mill found mill with Move\n");
                        getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
                        return true;
                    }                           
                }  
            } 
        }
    }
    return false;
}

bool calculateRandomMove(char *returnMove, bool toSet) {
    if (debugAusgabe) printf("Calculate Random Move\n");
    bool randomFound = false;
    srand(time(NULL));
    int counter = 0;
    int ring;
    int positionInRing;
    
    char oldPos[2] = "\0";
    
    while(!randomFound) {
        if(!toSet) {
            if (debugAusgabe) printf("All Pieces Set -> Move\n");
            int randOwnPiece = rand() % 9;
            counter++;
            if (counter > 100){
                for ( int ownPiece = 0; ownPiece < 9; ownPiece++) {
                    if(shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].isSet
                    && hasFreeNeighbor(shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID)) {
                        printf("randommove Backup Schranke\n");
                        oldPos[0] = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].position[0];
                        oldPos[1] = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].position[1];
                
                        int possibleDirections = 0;
                        if (positionInRing == 1 || positionInRing == 3 || positionInRing == 5 || positionInRing == 7 ) {
                        possibleDirections = 4;
                        } else {
                            possibleDirections = 2;
                        }
                
                        for ( int direction = 0; direction < possibleDirections; direction++) {
                            ring = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID[0];
                            positionInRing = shmdata->players[shmdata->clientPlayerId].piecelist[ownPiece].positionID[1];
                            
                            if(direction == 0) { //Rechts
                                if (positionInRing == 7) {
                                    positionInRing = 0;
                                } else {
                                    positionInRing = positionInRing + 1;
                                }
                            } else if (direction == 1) { //Links
                                if (positionInRing == 0) {
                                    positionInRing = 7;
                                } else {
                                    positionInRing = positionInRing - 1;
                                }
                            } else if (direction == 2){ //Wenn möglich hoch
                                if (ring != 0) {//nicht Aussen
                                    ring = ring -1;
                                } else {
                                    ring = ring +1;
                                }
                            } else { //Wenn möglich runter
                                if (ring != 2) {//nicht Innen
                                    ring = ring +1;
                                } else {
                                    ring = ring -1;
                                }
                            }
                        
                            if (shmdata->playingField[ring][positionInRing] == '0'){
                                        
                                getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
                                return true;
                            }                           
                        }  
                    } 
                }
            }
            if(shmdata->players[shmdata->clientPlayerId].piecelist[randOwnPiece].isSet
                && hasFreeNeighbor(shmdata->players[shmdata->clientPlayerId].piecelist[randOwnPiece].positionID)) {
                    oldPos[0] = shmdata->players[shmdata->clientPlayerId].piecelist[randOwnPiece].position[0];
                    oldPos[1] = shmdata->players[shmdata->clientPlayerId].piecelist[randOwnPiece].position[1];
                    if (debugAusgabe) printf("%c%c will move\n", oldPos[0], oldPos[1]);
                    
                    int direction;
                    
                    while(!randomFound) {
                        ring = shmdata->players[shmdata->clientPlayerId].piecelist[randOwnPiece].positionID[0];
                        positionInRing = shmdata->players[shmdata->clientPlayerId].piecelist[randOwnPiece].positionID[1];
                        
                        if (positionInRing == 1 || positionInRing == 3 || positionInRing == 5 || positionInRing == 7 ) {
                            direction = rand() % 4;
                            if(direction == 0) { //Rechts
                                if (positionInRing == 7) {
                                    positionInRing = 0;
                                } else {
                                    positionInRing = positionInRing + 1;
                                }
                            } else if (direction == 1) { //Links
                                if (positionInRing == 0) {
                                    positionInRing = 7;
                                } else {
                                    positionInRing = positionInRing - 1;
                                }
                            } else if (direction == 2){ //Wenn möglich hoch
                                if (ring != 0) {//nicht Aussen
                                    ring--;
                                } else {
                                    ring++;
                                }
                            } else { //Wenn möglich runter
                                if (ring != 2) {//nicht Innen
                                    ring++;
                                } else {
                                    ring--;
                                }
                            }
                        } else {
                            direction = rand() % 2;
                            if(direction == 0) { //Rechts
                                if (positionInRing == 7) {
                                    positionInRing = 0;
                                } else {
                                    positionInRing = positionInRing + 1;
                                }
                            } else { //Links
                                if (positionInRing == 0) {
                                    positionInRing = 7;
                                } else {
                                    positionInRing = positionInRing - 1;
                                }
                            }
                        }
                        
                        if ((shmdata->playingField[ring][positionInRing] == '0')) {
                            randomFound = true;
                        }                           
                            
                    }
                    getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
                    return true;
            }
        } else {
            ring = rand() % 3;
            positionInRing = rand() % 8;
            
            if(shmdata->playingField[ring][positionInRing] == '0') {
                getNameOfPosition(toSet, oldPos, ring, positionInRing, returnMove);
                return true;
            } 
        } 
    }
    return false;
}

void getNameOfPosition(bool toSet, char oldPiecPos[2], int ring, int positionInRing, char *returnMove) {   
    if(toSet) { 
        if (debugAusgabe) printf("Build Set String from:");
        switch(ring) {
            case 0:
                returnMove[0] = 'A';
                break;
            case 1:
                returnMove[0] = 'B';
                break;
            case 2:
                returnMove[0] = 'C';
                break;
        }
        
        returnMove[1] = positionInRing + '0';
    }
    else {
        if (debugAusgabe) printf("Build Move String\n");
        returnMove[0]= oldPiecPos[0];
        returnMove[1]= oldPiecPos[1];
        returnMove[2]= ':';
        
        switch(ring) {
            case 0:
                returnMove[3] = 'A';
                break;
            case 1:
                returnMove[3] = 'B';
                break;
            case 2:
                returnMove[3] = 'C';
                break;
        }
        
        returnMove[4] = positionInRing + '0';
    }
}

bool hasFreeNeighbor(int positionID[2]) {
    if (debugAusgabe) printf("Has free neighbor: %i %i\n", positionID[0], positionID[1]);
               
    //Wenn mittlere Position im Ring, dann schaue Position in anschliesenen Ringen frei
    if (positionID[1] == 1 || positionID[1] == 3 || positionID[1] == 5 || positionID[1] == 7) {
        if (positionID[0] == 0) {//Aussen
            if (shmdata->playingField[positionID[0]+1][positionID[1]] == '0') {
                return true;
            }
        } else if (positionID[0] == 1) {//Mitte
            if((shmdata->playingField[positionID[0]+1][positionID[1]] == '0') || (shmdata->playingField[positionID[0]-1][positionID[1]] == '0')) {
                return true;
            } 
        } else {//Innen
            if (shmdata->playingField[positionID[0]-1][positionID[1]] == '0') {
                return true;
            }
        }
    }
    
    //Schaue ob im Ring Rechts und links was frei ist 
    
    if (positionID[1] == 7) {
        if((shmdata->playingField[positionID[0]][0] == '0') || (shmdata->playingField[positionID[0]][6] == '0')) {
            return true;
        } else {
            return false;
        }
    } else if (positionID[1] == 0) {
        if((shmdata->playingField[positionID[0]][1] == '0') || (shmdata->playingField[positionID[0]][7] == '0')) {
            return true;
        } else {
            return false;
        }
    } else {
        if((shmdata->playingField[positionID[0]][positionID[1] + 1] == '0') || (shmdata->playingField[positionID[0]][positionID[1] - 1] == '0')) {
            return true;
        } else {
            return false;
        }
    }
}

bool isPieceInMuele(int ring, int positionInRing, char sign, int oldPosID[2]) {
    if (debugAusgabe) printf("Try is Piece in Mill %c: %i %i\n",sign, ring, positionInRing);
    if (debugAusgabe) printf("Wihout oldPiece %i %i\n", oldPosID[0], oldPosID[1]);
    if (positionInRing == 0 || positionInRing == 2 || positionInRing == 4 || positionInRing == 6 ) { //Ecke
        
        int posToTest;
        if (positionInRing == 7) {
            posToTest = 0;
        } else {
            posToTest = positionInRing + 1;
        }
        if (debugAusgabe) printf("Test positionInRing %i %i:if %c == %c && nich Ring %i\n", ring, posToTest, shmdata->playingField[ring][posToTest], sign, oldPosID[1]);
        if (shmdata->playingField[ring][posToTest] == sign && posToTest != oldPosID[1]) {
            if (debugAusgabe) printf("next positionInRing is Set\n");
            if (positionInRing == 7) {
                posToTest = 0;
            } else {
                posToTest = posToTest + 1;
            }
            if (debugAusgabe) printf("Test positionInRing %i %i:if %c == %c && nich Ring %i\n", ring, posToTest, shmdata->playingField[ring][posToTest], sign, oldPosID[1]);
            if (shmdata->playingField[ring][posToTest] == sign && posToTest != oldPosID[1]) {
                if (debugAusgabe) printf("next positionInRing is Set -> return in Mühle\n");
                return true;
            }
        }
        
        if (positionInRing == 0) {
            posToTest = 7;
        } else {
            posToTest = positionInRing - 1;
        }
        
        if (debugAusgabe) printf("Test positionInRing %i %i:if %c == %c && nich Ring %i\n", ring, posToTest, shmdata->playingField[ring][posToTest], sign, oldPosID[1]);
        if (shmdata->playingField[ring][posToTest] == sign && posToTest != oldPosID[1]) {
            if (debugAusgabe) printf("next positionInRing is Set\n");
            
            if (posToTest == 0) {
                posToTest = 7;
            } else {
                posToTest = posToTest - 1;
            }
        if (debugAusgabe) printf("Test positionInRing %i %i:if %c == %c && nich Ring %i\n", ring, posToTest, shmdata->playingField[ring][posToTest], sign, oldPosID[1]);
            if (shmdata->playingField[ring][posToTest] == sign && posToTest != oldPosID[1]) {
                if (debugAusgabe) printf("next positionInRing is Set -> return in Mühle\n");
                return true;
            }
        }
    } else { //Innen
        if (debugAusgabe) printf("Piece is in Middle of Ring\n");
        
        int posToTest;
        if (positionInRing == 7) {
            posToTest = 0;
        } else {
            posToTest = positionInRing + 1;
        }
        if (debugAusgabe) printf("Test positionInRing %i %i:if %c == %c && nich Ring %i\n", ring, posToTest, shmdata->playingField[ring][posToTest], sign, oldPosID[1]);
        if (shmdata->playingField[ring][posToTest] == sign && posToTest != oldPosID[1]) {
            if (debugAusgabe) printf("next positionInRing is Set\n");
            
            if (positionInRing == 0) {
                posToTest = 7;
            } else {
                posToTest = positionInRing - 1;
            }
            if (debugAusgabe) printf("Test positionInRing %i %i:if %c == %c && nich Ring %i\n", ring, posToTest, shmdata->playingField[ring][posToTest], sign, oldPosID[1]);
            if (shmdata->playingField[ring][posToTest] == sign && posToTest != oldPosID[1]) {
                if (debugAusgabe) printf("next positionInRing is Set -> return in Mühle\n");
                return true;
            }
        }
        
        if (ring == 2) {
            posToTest = 0;
        } else {
            posToTest = ring + 1;
        }
        if (debugAusgabe) printf("Test Ring %i %i:if %c == %c && nich positionInRing %i\n", posToTest, positionInRing, shmdata->playingField[ring][posToTest], sign, oldPosID[0]);
        if (shmdata->playingField[posToTest][positionInRing] == sign && posToTest != oldPosID[0]) {
            if (debugAusgabe) printf("next Ring is Set\n");
            if (ring == 2) {
                posToTest = 0;
            } else {
                posToTest = posToTest + 1;
            }
            if (debugAusgabe) printf("Test Ring %i %i:if %c == %c && nich positionInRing %i\n", posToTest, positionInRing, shmdata->playingField[ring][posToTest], sign, oldPosID[0]);
            if (shmdata->playingField[posToTest][positionInRing] == sign && posToTest != oldPosID[0]) {
                if (debugAusgabe) printf("next Ring is Set -> return in Mühle\n");
                return true;
            }
        }
    }
    return false;
}
