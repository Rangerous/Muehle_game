#pragma once

#include <stdbool.h>

typedef struct _Piece {
    bool isSet;
    bool isCaptured;
    char position[2];   //like: A1
    int positionID[2];  //like: 01
} Piece;

Piece initPiece();
