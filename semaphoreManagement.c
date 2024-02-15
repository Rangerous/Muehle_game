#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
extern bool debugAusgabe;

struct sembuf semaphore;

int createSemaphore(){
    int semid;
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0770);

    if (semid < 0){
        perror("Fehler bei createSemaphore: \n" );
        return -1;
    } else {
        if(debugAusgabe == true) printf("Semaphore angelegt \n");
        if(debugAusgabe == true) printf("Semaphore-ID: %d \n", semid);
        if (semctl(semid,0, SETVAL, (int) 1) == -1){
                        perror ("semctl fehler: ");
                };
    }
    return semid;
}

int wait_semaphore (int sid){
    semaphore.sem_op = -1;
    semaphore.sem_flg = SEM_UNDO;
    if (semop(sid, &semaphore,1) == -1){
        perror("wait semop: ");
    }
    return 0;
}

int signal_semaphore (int sid){
    semaphore.sem_op = 1;
    semaphore.sem_flg = SEM_UNDO;
    if (semop(sid, &semaphore,1)==-1){
        perror("signal semop: ");
    }
    return 0;
}
