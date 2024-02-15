#include <stdio.h>
#include <stdlib.h>
#include "memoryManagement.h"

void spielstandAusgabe() {
    char a[8] = "++++++++";
    char b[8] = "++++++++";
    char c[8] = "++++++++";
    char aVor[8] = " -----  "; 
    char bVor[8] = " ----- -";
    char cVor[8] = " -- -- -";
    char aNach[8] = "--   ---";
    char bNach[8] = "-- - ---";
    char cNach[8] = "-- - -- ";
    int i = 0;
    
    for(int k=0;k<3;k++){
        for(int j=0;j<8;j++){
            if (shmdata -> playingField [k][j] != '0'){
                switch (k){
                    case 0:
                        a[j] = shmdata->playingField [k][j];
                        break;
                    case 1:
                        b[j] = shmdata->playingField [k][j];
                        break;
                    case 2:
                        c[j] = shmdata->playingField [k][j];
                        break;
                }
            }
        }                        
    }
    

    while(i<=7){
        if( a[i] != '+' ){
            aVor[i] = '(';
            aNach[i] = ')';
        }
        if( b[i] != '+' ){
            bVor[i] = '(';
            bNach[i] = ')';
        }
        if( c[i] != '+' ){
            cVor[i] = '(';
            cNach[i] = ')';
        }
        i++;
    }
    
    
    printf("%c%c%c---------%c%c%c---------%c%c%c\n", aVor[0], a[0], aNach[0], aVor[1], a[1], aNach[1], aVor[2], a[2], aNach[2]);
    printf(" |           |           |\n");
    printf(" |  %c%c%c-----%c%c%c-----%c%c%c  |\n", bVor[0], b[0], bNach[0], bVor[1], b[1], bNach[1], bVor[2], b[2], bNach[2]);
    printf(" |   |       |       |   |\n");
    printf(" |   |  %c%c%c-%c%c%c-%c%c%c  |   |\n", cVor[0], c[0], cNach[0], cVor[1], c[1], cNach[1], cVor[2], c[2], cNach[2]);
    printf(" |   |   |       |   |   |\n");
    printf("%c%c%c-%c%c%c-%c%c%c     %c%c%c-%c%c%c-%c%c%c\n", aVor[7], a[7], aNach[7], bVor[7], b[7], bNach[7], cVor[7], c[7], cNach[7], cVor[3], c[3], cNach[3], bVor[3], b[3], bNach[3], aVor[3], a[3], aNach[3]);
    printf(" |   |   |       |   |   |\n");
    printf(" |   |  %c%c%c-%c%c%c-%c%c%c  |   |\n", cVor[6], c[6], cNach[6], cVor[5], c[5], cNach[5], cVor[4], c[4], cNach[4]);
    printf(" |   |       |       |   |\n");
    printf(" |  %c%c%c-----%c%c%c-----%c%c%c  |\n", bVor[6], b[6], bNach[6], bVor[5], b[5], bNach[5], bVor[4], b[4], bNach[4]);
    printf(" |           |           |\n");
    printf("%c%c%c---------%c%c%c---------%c%c%c\n", aVor[6], a[6], aNach[6], aVor[5], a[5], aNach[5], aVor[4], a[4], aNach[4]);
}
