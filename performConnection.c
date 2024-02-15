#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include "performConnection.h"
#include "playerHandling.h"
#include "memoryManagement.h"
#include "piece.h"
#include "spielstandAusgabe.h"
#include "config.h"

#define VERSION "VERSION 3.0\n"
#define PLAYER "PLAYER "
#define THINKING "THINKING\n"
#define OKWAIT "OKWAIT\n"
#define BUF_SIZE 512

extern bool debugAusgabe;
char buffer[BUF_SIZE];
bool prologRunning = true;
bool gamePhaseRunning = false;
int line = 0;
int mySocket;
shmContent_t* sharedMemory;

bool checkSocket(int);
int readLineFromServer();
bool isPositiveMsg();
void getWordAtPosition(char*, char*, int);
bool getWordsFromIndex(char*, char*, int) ;
bool handleMove();
bool handlePiecelist();
bool handleGameOver();
bool readPlayers();
void updatePlayingfield();

int performConnection(int sock, int sharedMemoryID) {
    if ((sharedMemory = shmat (sharedMemoryID, NULL, 0)) == (void *)-1) {
		printf("Something went wrong with shared Memory! %s\n", strerror(errno));
		printf("Something went wrong with shared Memory! %s\n", strerror(errno));
        return EXIT_FAILURE;
    } else {
        if(debugAusgabe == true) printf("Connector is connected to shared Memmory\n");
	}

    if(!checkSocket(sock)) {
		perror ("Connector is not connected with socket\n");
        return EXIT_FAILURE;
    } else {
        if(debugAusgabe == true) printf("Connector is connected to socket\n");
        mySocket = sock;
    }
    
    
    while (prologRunning) {

        if (readLineFromServer() < 1) {
            return EXIT_FAILURE;
        }

        if (isPositiveMsg()) {
            line++;
        } else {
            line = -1;
        }
        
        if(debugAusgabe == true) printf("Reached line: %i\n", line); // DEBUG

        switch (line) {
            case 1:
                write(mySocket, VERSION, strlen(VERSION));
                break;
            case 2:{
                char* answerHappyWithAI = "NO ";

                //write(mySocket, answerHappyWithAI, strlen(answerHappyWithAI));
                if(debugAusgabe == true) printf("sent nothing back to answerHappyWithAI: %s\n", answerHappyWithAI);
                break;
                
            }
            case 3:{                
                char answerGameID[16] = "ID ";
                strcat(answerGameID, sharedMemory->gameID);
                strcat(answerGameID, "\n");

                write(mySocket, answerGameID, strlen(answerGameID));
                if(debugAusgabe == true) printf("sent answerGameID back: %s\n", answerGameID);
                break;
            }
            case 4: {
                char gameKindName[32];
                getWordAtPosition(buffer, gameKindName, 3);
                if (!(strcmp(gameKindName, config.gameKindName) == 0)) {
                    printf("Server-gamekind-name (\"%s\") does not equal config-gamekind-name (\"%s\") - aborting!\n", gameKindName, config.gameKindName);
                    return EXIT_FAILURE;
                }
                break;
            }
            case 5:{
                char gameName[16];
                if (!getWordsFromIndex(buffer, gameName, 1)) {
                    printf("GetWordsFromIndex failed!\n");
                    return EXIT_FAILURE;
                }

                if(debugAusgabe == true) printf("GetWordsFromIndex successful, %s\n", gameName);
                strcpy(sharedMemory->gamename, gameName);
                printf("gameName: %s\n", sharedMemory->gamename); // DEBUG

                char answerPlayerID[16] = "PLAYER ";

                if(debugAusgabe == true) printf("Desired playerID %d\n", sharedMemory->desiredPlayerId);
                
                if (sharedMemory->desiredPlayerId != -1) {
                    char playerIDString[16];
                    sprintf(playerIDString, "%d", sharedMemory->desiredPlayerId);
                    strcat(answerPlayerID, playerIDString);
                }
                
                strcat(answerPlayerID, "\n");
                if(debugAusgabe == true) printf("answerPlayer: %s\n", answerPlayerID);
                write(mySocket, answerPlayerID, strlen(answerPlayerID));
                
                break;
            }
            case 6:{
                char bufferCopy[64];
                strcpy(bufferCopy, buffer);

                char playerNumberString[2];
                getWordAtPosition(buffer, playerNumberString, 3);

                int playerNumber = atoi(playerNumberString);
                sharedMemory->clientPlayerId = playerNumber;

                if (debugAusgabe) printf("playerNumberAsString: %s\n", playerNumberString); // DEBUG
                if (debugAusgabe) printf("playerNumber: %d\n", sharedMemory->clientPlayerId); // DEBUG

                char playerName[16];
                if(debugAusgabe == true) printf("bufferCopy: %s\n", bufferCopy);
                if (!getWordsFromIndex(bufferCopy, playerName, 3)) {
                    printf("getWordsFromIndex failed!\n");
                    return EXIT_FAILURE;
                }

                strcpy(sharedMemory->players[sharedMemory->clientPlayerId].name, playerName);


                
                sharedMemory->players[0].sign = '1';
                sharedMemory->players[1].sign = '2';
                

               printf("playerName: %s\n", sharedMemory->players[sharedMemory->clientPlayerId].name); // DEBUG
                
                
                break;
            }
            case 7: {
                char playerCountString[2];
                getWordAtPosition(buffer, playerCountString, 3);

                int playerCount = atoi(playerCountString);
                sharedMemory->nplayers = playerCount;

                if(debugAusgabe == true) printf("playerCount: %d\n", sharedMemory->nplayers); // DEBUG

                readPlayers(sharedMemory);
                prologRunning = false;
                gamePhaseRunning = true;
                if(debugAusgabe == true) printf("Prolog phase finished\n");
                break;
            } 
            case -1:
                printf("ERROR: \"%s\"!\n", buffer);
                prologRunning = false;
                gamePhaseRunning = false;
                return EXIT_FAILURE;
                break;
            default:
                printf("Prolog phase not defined\n");
                prologRunning = false;
                gamePhaseRunning = false;
                return EXIT_FAILURE;
                break;
        }
    }

    while (gamePhaseRunning) {

        if(debugAusgabe == true) printf("Reached game phase\n"); // DEBUG

        if (readLineFromServer() < 1) {
            return EXIT_FAILURE;
        }

        if (!isPositiveMsg()) {
            return EXIT_FAILURE;
        }

        char bufferCopy[64];
        strcpy(bufferCopy, buffer);

        char keyword[16];
        getWordAtPosition(buffer, keyword, 2);
        if(debugAusgabe == true) printf("Keyword is: %s\n", keyword);

        if (strcmp("WAIT", keyword) == 0) {
            if(debugAusgabe == true) printf("Entered WAIT branch\n");
            write(mySocket, OKWAIT, strlen(OKWAIT));
             if(debugAusgabe == true) printf("Answered OKWAIT\n");
        } else if (strcmp("MOVE", keyword) == 0) {            
            char maxMoveTimeString[16];
            getWordAtPosition(bufferCopy, maxMoveTimeString, 3);
            int maxMoveTime = atoi(maxMoveTimeString);
            sharedMemory->maxMoveTime = maxMoveTime;

            if(debugAusgabe == true) printf("maxMoveTime: %d\n", sharedMemory->maxMoveTime);
        } else if (strcmp("CAPTURE", keyword) == 0) {
            char capturedPieceCountString[16];
            getWordAtPosition(bufferCopy, capturedPieceCountString, 3);
            int capturedPieceCount = atoi(capturedPieceCountString);
            
            if( 0 < capturedPieceCount) {
                sharedMemory->capturedPieceCount = capturedPieceCount;
                sharedMemory->shouldCapture = true;
            } 

            if(debugAusgabe == true) printf("capturedPieceCount: %d\n", sharedMemory->capturedPieceCount);
            
        } else if (strcmp("PIECELIST", keyword) == 0) {
            char data[16];
            getWordAtPosition(bufferCopy, data, 3);
            char* pieceCountString = data + 2;
            int pieceCount = atoi(pieceCountString);
            sharedMemory->playerPieceCount = pieceCount;

            if(debugAusgabe == true) printf("pieceCount: %d\n", sharedMemory->playerPieceCount);

            if (!handlePiecelist(sharedMemory)) {
                printf("Something went wrong in handlePiecelist().\n");
                gamePhaseRunning = false;
                return EXIT_FAILURE;
            }

            printf("Sending THINKING\n"); // DEBUG
            write(mySocket, THINKING, strlen(THINKING));

            if (!handleMove(sharedMemory)) {
                if(debugAusgabe == true) printf("Something went wrong in handleMove().\n");
                gamePhaseRunning = false;
                return EXIT_FAILURE;
            } 
        } else if ((strcmp("GAMEOVER", keyword) == 0)) {
            if(debugAusgabe == true) printf("Entered GAMEOVER branch\n");
            if (!handleGameOver(sharedMemory)) {
                if(debugAusgabe == true) printf("Something went wrong in hadleGameOver().\n");
                return EXIT_FAILURE;
            } 
            gamePhaseRunning = false;
        }     
    } 
    
    return EXIT_SUCCESS;
}

bool checkSocket(int sock) {
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    retval = select(sock + 1, &rfds, NULL, NULL, &tv);

    if (retval == -1) {
        perror("select()\n");
        return false;
    } else if (retval) {
        if(debugAusgabe == true) printf("Data is available now.\n"); 
        return true;
    } else {
        if(debugAusgabe == true) printf("No data within three seconds.\n");
        return false;
    }
}

bool checkPipe(){
    fd_set rfdp;
    struct timeval timeout;
    int retvalPipe;

    FD_ZERO(&rfdp);
    FD_SET(fd[0],&rfdp);
    
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if ((retvalPipe = select(fd[0]+1,&rfdp,NULL,NULL,&timeout))== -1){
        perror("select(fd)\n");
        return false;
    } else if (retvalPipe > 0) {
        if(debugAusgabe == true) printf("Data on Pipe is available now.\n"); 
        return true;
    } else {
        printf("No data on pipe within 3 seconds.\n");
        return false;
    }
    
}

int readLineFromServer() {
    
    int bytesloaded = 0;
    char buf[2];
    
    // Clear buffer
    memset(&buffer[0], 0, sizeof(buffer));
    //printf("Buffer's content after clearing: %s\n", buffer); // Debug

    while(read(mySocket, buf, 1) > 0) {
        buf[1] = '\0';
        bytesloaded += strlen(buf);

        if(buf[0] == '\n') {
            if(debugAusgabe == true) printf("Buffer's content after reading until NewLine: %s\n", buffer); // Debug
            return bytesloaded;
        } else {
            strcat(buffer, buf);
        }
    }

    if(debugAusgabe == true) printf("Buffer's content after reading until End: %s\n", buffer); // Debug
    return bytesloaded;
}

bool isPositiveMsg() {
    char firstLetter = buffer[0];

    if (strcmp(&firstLetter,"-") >= 0) {
        return false;
    } else {
        return true;
    }
}

void getWordAtPosition(char* str, char* dest, int n) {
    char* p = strtok(str, " ");

    for (int i = 1; i < n; i++) {
        p = strtok(NULL, " ");
    } 

    if (p != NULL) {
        strcpy(dest, p);
    } 
} 

bool getWordsFromIndex(char* str, char* dest, int n) {
    int wordCount = 0;
    int i = 0;

    if (strlen(str) < n) {
        return false;
    }

    while(wordCount < n) {
        if (str[i] == ' ') {
            wordCount++;
        }
        i++;
    }

    strcpy(dest, buffer + i);
    return true;
}

bool handleMove() {

    if(debugAusgabe == true) printf("+++ HANDLING MOVE\n");

    bool success = false;
    bool handlingMove = true;
    int movePhase = 0;

    while (handlingMove) {
        if (readLineFromServer() < 1) {
            return EXIT_FAILURE;
        }

        if (isPositiveMsg()) {
            movePhase++;
        } else {
            movePhase = -1;
        }

        if(debugAusgabe == true) printf("Move phase %d\n", movePhase);
        
        switch (movePhase) {
            case 1:
            
                // OKTHINK empfangen
                if(debugAusgabe == true) printf("Sending PLAY command\n"); // DEBUG
                
                sharedMemory->shouldThink = true;                
                kill (getppid(),SIGUSR1 );
                
                int pipeLenght = 6;
                char pipeMsg[6];
                // Leseseite auslesen
                
                if(!checkPipe()) {
		            perror ("Timeout: no data on pipe\n");
                    return EXIT_FAILURE;
                } else {
                    if(debugAusgabe == true) printf("data on pipe is going to be read\n");
                    
                }


                if (read(fd[0], pipeMsg, pipeLenght) == -1  ) {
                    perror("Error while reading pipe");
                    return EXIT_FAILURE;
                }
                
                if(debugAusgabe == true) printf("PipeMsg: %s\n", pipeMsg);
                char answerMsg[11] = "PLAY ";
                strcat(answerMsg, pipeMsg);
                strcat(answerMsg, "\n");
                if(debugAusgabe == true) printf("Send Message: %s\n", answerMsg);
                
                write(mySocket, answerMsg, strlen(answerMsg));
                //BUG: name und playerID passen nicht zueinander
                //printf("playerName: %s\n", sharedMemory->players[sharedMemory->clientPlayerId].name); 
                printf("%s (Player %d) moves: %s\n",sharedMemory->players[sharedMemory->clientPlayerId].name, sharedMemory->clientPlayerId +1,answerMsg);
                printf("Waiting for %s (Player %d)\n", sharedMemory->players[(sharedMemory->clientPlayerId +1)%2 ].name,sharedMemory->players[(sharedMemory->clientPlayerId +1)%2].ID +1);
                break;
            case 2:
                // MOVEOK empfangen
                handlingMove = false;
                success = true;
                break;
            case -1:
                handlingMove = false;
                if(debugAusgabe == true) printf("Received negative message in handlingMove()\n");
                break;
            default:
                handlingMove = false;
                if(debugAusgabe == true) printf("Undefined move phase: %d\n", movePhase);
                break;
        }
    } 
    
    return success;
} 


bool handlePiecelist() {
    if(debugAusgabe == true) printf("+++ HANDLING PIECELIST\n");

    bool handlingPiecelist = true;
    
    sharedMemory->players[0].capturedPieces = 0;
    sharedMemory->players[1].capturedPieces = 0;
    
    while (handlingPiecelist) {
        if (readLineFromServer() < 1) {
            return EXIT_FAILURE;
        }

        if (!isPositiveMsg()) {
            return false;
        } 

        char bufferCopy[64];
        strcpy(bufferCopy, buffer);

        char keyword[16];
        getWordAtPosition(buffer, keyword, 2);
        if(debugAusgabe == true) printf("Keyword is: %s\n", keyword);
 
        if (strcmp("ENDPIECELIST", keyword) == 0) {
            if(debugAusgabe == true) printf("Done handling piecelist\n");  // DEBUG
            handlingPiecelist = false;
        } else {
            if(debugAusgabe == true) printf("Handling piece: %s\n", keyword);

            // Subtracting 48 from a char yields the int
            int playerNumber = keyword[5] - 48;
            int pieceNumber = keyword[7] - 48;
            char position[2];
            getWordAtPosition(bufferCopy, position, 3);

            if(debugAusgabe == true) printf("playernumber: %d\n", playerNumber);
            if(debugAusgabe == true) printf("pieceNumer: %d\n", pieceNumber);
            if(debugAusgabe == true) printf("position: %s\n", position);

            Piece piece = initPiece();
            if (strcmp(position, "A") == 0) {
                piece.isSet = false;
            } else if (strcmp(position, "C") == 0) {
                piece.isCaptured = true;
                piece.isSet = false;
                sharedMemory->players[playerNumber].capturedPieces = sharedMemory->players[playerNumber].capturedPieces + 1;
            } else {
                strcpy(piece.position, position);
                piece.isSet = true;
                
                if (position[0] == 'A') {
                    piece.positionID[0] = 0;
                } else if (position[0] == 'B') {
                    piece.positionID[0] = 1;
                } else {
                    piece.positionID[0] = 2;
                }
                
                piece.positionID[1] = position[1] - '0';
            }

            sharedMemory->players[playerNumber].piecelist[pieceNumber] = piece;

            if(debugAusgabe == true) printf("PIECE %d from player %d\n", pieceNumber, playerNumber);
            if(debugAusgabe == true) printf("isSet: %d\n", sharedMemory->players[playerNumber].piecelist[pieceNumber].isSet);
            if(debugAusgabe == true) printf("isCaptured: %d\n", sharedMemory->players[playerNumber].piecelist[pieceNumber].isCaptured);
            if(debugAusgabe == true) printf("Captured Count: %d\n", sharedMemory->players[playerNumber].capturedPieces);
            if(debugAusgabe == true) printf("position: %s\n", sharedMemory->players[playerNumber].piecelist[pieceNumber].position);
            if(debugAusgabe == true) printf("positionID: %i, %i\n", sharedMemory->players[playerNumber].piecelist[pieceNumber].positionID[0], sharedMemory->players[playerNumber].piecelist[pieceNumber].positionID[1]);
        } 
    }
    
    updatePlayingfield();
    spielstandAusgabe(); 
    return true;
}  

bool handleGameOver() {
    bool success = false;
    bool handlingGameOver = true;
    int gameOverPhase = 0;
    char keyword[16];
    while(handlingGameOver) {
        if (readLineFromServer() < 1) {
            handlingGameOver = false;
        }

        if (isPositiveMsg()) {
            gameOverPhase++;
        } else {
            gameOverPhase = -1;
        }

        if (debugAusgabe) printf("GameOver phase %d\n", gameOverPhase);
        
        switch (gameOverPhase) {
            case 1:
                printf("Gameover: have a look at the gameboard in the end! \n");
                if (!handlePiecelist()) {
                    success = false;
                    handlingGameOver = false;
                } 
                break;
            case 2: {
                //char keyword[16];
                getWordAtPosition(buffer, keyword, 3);
                if (debugAusgabe) printf("Keyword is: %s\n", keyword);

                if (strcmp("Yes", keyword) == 0) {       
                    sharedMemory->players[0].won = true;
                } else {
                    sharedMemory->players[0].won = false;
                }

                if (debugAusgabe) printf("player 0 won: %d\n", sharedMemory->players[0].won);

                break;
            }
            case 3: {
                char keyword[16];
                getWordAtPosition(buffer, keyword, 3);
                if (debugAusgabe) printf("Keyword is: %s\n", keyword);

                if (strcmp("Yes", keyword) == 0) {       
                    sharedMemory->players[1].won = true;
                } else {
                    sharedMemory->players[1].won = false;
                }

                if (debugAusgabe) printf("player 1 won: %d\n", sharedMemory->players[1].won);

                break;
            }
            case 4:
                // QUIT empfangen
                handlingGameOver = false;
                success = true;
                break;
            case -1:
                handlingGameOver = false;
                printf("Received negative message in handlingGameOver()\n");
                break;
            default:
                handlingGameOver = false;
                printf("Undefined move phase: %d\n", gameOverPhase);
                break;
        } 
    } 
    
    if (sharedMemory->players[0].won && sharedMemory->players[1].won){
        printf("A fair draw!\n");
    } else if (sharedMemory->players[0].won && !sharedMemory->players[1].won){
        printf("Oh no, %s (Player %d), you lost...\n", sharedMemory->players[1].name, sharedMemory->players[1].ID +1);
        printf("Congrats %s (Player %d), you won! \n", sharedMemory->players[0].name, sharedMemory->players[0].ID + 1);
    } else if (sharedMemory->players[1].won && !sharedMemory->players[0].won){
        printf("Oh no %s (Player %d), you lost...\n", sharedMemory->players[0].name, sharedMemory->players[0].ID +1);
        printf("Congrats %s (Player %d), you won! \n", sharedMemory->players[1].name, sharedMemory->players[1].ID + 1);
    }
    
    printf("\nWarum mögen Frauen Objektorientiert-Programmierer?\n \nWeil Sie Klasse haben.\n");

    return success;
} 

bool readPlayers() {
    bool done = false;

    while(!done) {
        if(debugAusgabe == true) printf("INSIDE READPLAYERS\n");
        if (readLineFromServer() < 1) {
            printf("readPlayers(): Readline failed!\n");
            return false;
        }
    
        if (!isPositiveMsg()) {
            printf("readPlayers(): Message is negative\n");
            return false;
        } 

        if(debugAusgabe == true) printf("buffer is: %s\n", buffer);
        if (strcmp("+ ENDPLAYERS", buffer) == 0) {
            if(debugAusgabe == true) printf("readPlayers(): ENDPLAYERS read\n");
            done = true;
            return true;
        } else {
            char bufferCopy[64];
            strcpy(bufferCopy, buffer);

            char playerNumberString[2]; 
            getWordAtPosition(buffer, playerNumberString, 2);

            int playerNumber = atoi(playerNumberString);
            if (debugAusgabe) printf("playernumber zeile 642: %d\n",playerNumber);
            if (debugAusgabe) printf("playerID 642: %d\n", sharedMemory->players[playerNumber].ID);
            sharedMemory->players[playerNumber].ID = playerNumber;
            if (debugAusgabe) printf("Variable playernumber: %d\n",playerNumber);
            if (debugAusgabe) printf("playerID 644: %d\n", sharedMemory->players[playerNumber].ID);
            if(debugAusgabe == true) printf("playerNumber: %d\n", sharedMemory->players[playerNumber].ID); // DEBUG

            char nameAndReady[32];

            if (!getWordsFromIndex(bufferCopy, nameAndReady, 2)) {
                printf("getWordsFromIndex failed!\n");
                return false;
            } else {
                if(debugAusgabe == true) printf("nameAndReady: %s\n", nameAndReady); // DEBUG
            }

            int nameAndReadyLenth = strlen(nameAndReady);
            char readyChar = nameAndReady[nameAndReadyLenth];
            bool ready = readyChar; // Char to int conversion
            sharedMemory->players[playerNumber].ready = ready;
            //printf("nameandready: %s\n",nameAndReady);
            nameAndReady[nameAndReadyLenth - 2] = '\0';
            //printf("name Zeile 660: %s\n", sharedMemory->players[sharedMemory->clientPlayerId].name);
            strcpy(sharedMemory->players[playerNumber].name, nameAndReady);

            //printf("name Zeile 662: %s\n", sharedMemory->players[sharedMemory->clientPlayerId].name);
            //printf("name playernumber Z 663: %s\n", sharedMemory->players[playerNumber].name);

            if(debugAusgabe == true) printf("ready player %d: %d\n", playerNumber, sharedMemory->players[playerNumber].ready); // DEBUG
            if(debugAusgabe == true) printf("name player %d: %s\n", playerNumber, sharedMemory->players[sharedMemory->clientPlayerId].name); // DEBUG
        }
    }


    return true;
}

void updatePlayingfield() {

    if(debugAusgabe) printf("+++ UPDATING PLAYINGFIELD\n");
    for(int i = 0; i<3; i++){
        for(int j =0;j<8;j++){
                sharedMemory->playingField[i][j] = '0';
        }
    }

    for (int i = 0; i < sharedMemory->nplayers; i++) {
        for (int j = 0; j < sharedMemory->playerPieceCount; j++) {

            if (sharedMemory->players[i].piecelist[j].isSet) {
                char* position = sharedMemory->players[i].piecelist[j].position;

                int positionInRing = position[1] - 48; // Subtracting '0' from a char yields the int
                int ringIndex;


                if (position[0] == 'A') {
                    ringIndex = 0;
                } else if (position[0] == 'B') {
                    ringIndex = 1;
                } else if (position[0] == 'C') {
                    ringIndex = 2;
                }

                sharedMemory->playingField[ringIndex][positionInRing] = sharedMemory->players[i].sign;

                if (debugAusgabe) {
                    printf("Set piece: \n");
                    printf("ring index: %d\n", ringIndex);
                    printf("position in ring: %d\n", positionInRing);
                    printf("value: %c\n", i + '0' + 1);
                } 
            }
        }
    }

    if(debugAusgabe) printf("+++ UPDATED PLAYINGFIELD\n");
}
