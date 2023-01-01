//Task di log
//Gestisce la scrittura su un file di log tramite una funzione utilizzabile dai vari task

#include "logTask.h"

int LOG_FD;


void mytest(void){
	printf("\nprova\n");
}

//Funzione che scrive i messaggi sul file di log, gestisce l'accesso concorrente al file con un semaforo 

void logMessage(char* msg, char* task_name){
	char final_msg[1024] = "task : ";
	
	//TODO inserire controllo su lunghezza messaggio
	
	strcat(final_msg, task_name);
	strcat(final_msg, "\t");	
	strcat(final_msg, msg);	
	strcat(final_msg, "\n");
	
	//acquisisco il semaforo e scrivo sul file
	if(semTake(SEM_LOG, WAIT_FOREVER) == ERROR){
		perror("\nErrore nell'acquisire semaforo di log");
		return;
	}
	
	if(write(LOG_FD, final_msg, strlen(final_msg)) < 0){
		perror("\nErrore nella scrittura di un log:");
	}
	//rilascio il semaforo
	if(semGive(SEM_LOG) == ERROR){
		perror("\nErrore nel rilasciae il semaforo di log");
		return;
	}
	
	
}



void logInit(void){
	//Crea file di log
	
	struct stat st = {0};
	//Controlla se esiste directory di log, altrimenti la crea
	if (stat("/usr/log", &st) == -1) {
	    if(mkdir("/usr/log", 00700) < 0){
	    	perror("\nErrore creazione directory di log:");
	    }
	}
	//Crea il file di log
	if ((LOG_FD = open("/usr/log/log.txt",O_RDWR | O_APPEND  | O_CREAT, 00700)) < 0){
		perror("\nErrore apertura file di log");
	}
	
	//creo il semaforo per regolare l'accesso al file di log
	SEM_LOG = semBCreate(SEM_Q_FIFO, SEM_FULL);
	
	logMessage("Messaggio log di prova", "tLog");
	
	
	close(LOG_FD);
	
}
