#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/sem.h>
#include <signal.h>
#include "config.h"
#include "server_connection.h"
#include "playerHandling.h"
#include "outputManagement.h"
#include "performConnection.h"
#include "memoryManagement.h"
#include "semaphoreManagement.h"
#include "think.h"

#define GAMEKINDNAME "NMMorris"

bool run = false;
char configFilename[60] = "client.conf";
void spielstandAusgabe();


void stop(int sig){
	if(debugAusgabe == true) printf("before closed\n");
	run = false;
	if(debugAusgabe == true) printf("After close\n");
}

int main(int argc, char* argv[]){
    
    int desiredPlayerId = -1;
	char gameID[13] = "";
    
    //spielstandAusgabe(); //momentan nur zum Testen ich weiß nicht wo das dann hingehört
    

	while(1){
		int result = getopt(argc, argv, "g:p:c:d");
		if(result == -1) break;
		switch(result){
			case '?':
				break;
			case ':':
                printErrorMessage("Fehlendes Argument!");
				break;
			case 'g':
				strcpy(gameID, optarg);
				if(debugAusgabe == true) printf("Game-ID: %s\n",gameID);
				break;
			case 'p':
				desiredPlayerId = atoi(optarg) - 1;
				if(debugAusgabe == true) printf("Desired Player-ID: %i\n",desiredPlayerId);
				break;
			case 'c':
                strcpy(configFilename, optarg);
                break;
            case 'd':
                debugAusgabe = true;
			default:
				break;
		}

	}


	if (readConfig(configFilename) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}
	
	
	int socket = createSocket(config.port, config.hostname);

    if(debugAusgabe == true) printf("Socket: %i\n", socket);
    

    if(socket > 0) {
        //printLineOnConsole("Perfom performConnection wird gestartet");     irreführend, da performconnection erst nach fork gestartet wird

        int shmid = createMemory();
		//shmContent_t *shmdata;
		int deleteShm = shmid;
		if(debugAusgabe == true) printf ("shmid in sysprakclient: %i \n", shmid);
		pid_t pid;
		int shmDetach;
        
		//Semaphore 
        int semid = createSemaphore();
		if(debugAusgabe == true) printf("%d \n",semid);
		unsigned short out[1];
		semctl(semid,0,GETALL,out);
		if(debugAusgabe == true) printf("Wert des Semaphors: %i \n",out[0]);
		wait_semaphore(semid);
		semctl(semid,0,GETALL,out);
		if(debugAusgabe == true) printf("Wert des Semaphors nach wait: %i \n",out[0]);
		signal_semaphore(semid);
		semctl(semid,0,GETALL,out);
		if(debugAusgabe == true) printf("Wert des Semaphors nach wait: %i \n",out[0]); //wird noch aufgeräumt, nur zum testen aktuell

        //Pipe erstellen    
        if (pipe (fd) < 0) {
            perror ("pipe");
            exit (EXIT_FAILURE);
        }


        if ((shmdata = shmat (shmid, NULL, 0)) == (void *)-1) {
            perror ("thinker shmat:");
        } else {
            if(debugAusgabe == true) printf ("shmid attached to thinker \n");
        }
        
		for (int i = 0; i < sizeof(shmdata->players)/sizeof(Player); i++) {
			shmdata->players[i] = initPlayer();
			if(debugAusgabe == true) printf("PlayerID: %d\n", shmdata->players[i].ID);
		}

        shmdata->desiredPlayerId = desiredPlayerId;
        shmdata->gameID = gameID;
		shmdata->shouldThink=false;

		for(int k=0;k<3;k++){
			for(int l=0;l<8;l++){
				shmdata->playingField [k][l]='0';
			}
		}
        
		if ((pid = fork()) < 0) {
			fprintf(stderr, "fork() error\n");
		} else if (pid == 0) {
			// Child process (connector)
			if(debugAusgabe == true) printLineOnConsole("Entered child process (connector)"); 
            
            // Schreibseite der Pipe schließen 
			close (fd[1]);
            
			if (performConnection(socket, shmid) == EXIT_SUCCESS) {
				if(debugAusgabe == true) printLineOnConsole("performConnection erfolgreich beendet");
			} else {
				if(debugAusgabe == true) printErrorMessage("performConnection unerwartet beendet");
			}

			if ((shmDetach = shmdt (shmdata)) == -1 ){
				perror ("child shm not detached:");
			} else {
				if(debugAusgabe == true) printf ("child shm detached \n");
			}
			
            if(debugAusgabe == true) printf("Child process stopped\n");
            kill(getppid(), SIGUSR2);
			close(0);
		} else {
			// Parent process (thinker)
			if(debugAusgabe == true) printLineOnConsole("Entered parent process (thinker) after fork");
			
			if(debugAusgabe == true) printf ("ThinkerID in Thinker: %i \n", pid);
            shmdata->thinkerid = pid;
			
            //Schließe lesenseite der Pipe
			close (fd[0]);
			
			signal(SIGUSR1, think);
			signal(SIGUSR2, stop);
			run = true;
			while(run)
			{
				pause();
			}
			
			if (waitpid(pid, NULL, 0) != pid)  {    
                perror (" Fehler beim Warten auf Kindprozess \n");
                return EXIT_FAILURE ;
            } else {
            if(debugAusgabe == true) printf("All child process stopped -> leave fork\n");
            }
			if ((shmDetach = shmdt (shmdata)) == -1){  //shmDetach nach waitpid, da sonst shm detached wird bevor connector startet
				perror ("thinker shm not detached: ");
			} else {
				if(debugAusgabe == true) printf ("thinker shm detached \n");
			}
			shmctl (deleteShm, IPC_RMID, NULL);
        }
		
    } else {
        printErrorMessage("Fehler bei Verbindungsaufbau!");
    }
    
    close(socket);
	if(debugAusgabe == true) printf("Socket is closed\n");
    return EXIT_SUCCESS;
}
