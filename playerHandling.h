#pragma once

#include <stdbool.h>
#include "piece.h"

#define MaxPlayer (2)

typedef struct _Player {
    char name [16];
    char sign;
    int ID;
    bool ready;
    bool won;
    Piece piecelist[9];
    int capturedPieces;
} Player;

Player initPlayer();
