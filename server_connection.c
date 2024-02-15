#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "server_connection.h"
#include <stdbool.h>
extern bool debugAusgabe;


int createSocket(char* portnumber, char* hostname){
        
    // Struktur addrinfo für getaddrinfo initialisieren
    struct addrinfo *result;
    struct addrinfo hints ;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL; 
    int sock;
    int s; 


    s = getaddrinfo (hostname, portnumber, &hints, &result);
    if (s == 0) {
        if(debugAusgabe == true) printf ("getaddrinfo geht \n");
    } else {
        perror ("getaddrinfo error: \n");
    }

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("sock error: \n");
        exit(1);
    } else {
        if(debugAusgabe == true) printf("sock läuft \n");
    }
        
    if (connect (sock, result->ai_addr, result->ai_addrlen ) == 0) {
       if(debugAusgabe == true) printf("Verbindung hergestellt \n");
    } else {
        printf("Erno: %d\n", errno);
        perror ("connect error: \n");
    }

    freeaddrinfo(result);
    return sock;
}

