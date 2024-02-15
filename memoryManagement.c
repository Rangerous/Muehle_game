#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdbool.h>
#include "memoryManagement.h"
#include "playerHandling.h"
extern bool debugAusgabe;


int createMemory () {
    int shmid;

    if ((shmid = shmget(IPC_PRIVATE, sizeof (shmContent_t), IPC_CREAT | 0666)) == -1){
        perror("error shmget:  \n");
    } else {
        if(debugAusgabe == true) printf ("shared memory erstellt \n");
        if(debugAusgabe == true) printf ("%i \n", shmid);
        return shmid;
    }

    return 0;
}
