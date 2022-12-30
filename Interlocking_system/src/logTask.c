//Task di log
//Gestisce la scrittura su un file di log tramite una funzione utilizzabile dai vari task

#include "logTask.h"


void logMessage(char* msg){
	
	
	if(write(LOG_FD, msg, strlen(msg)) < 0){
		perror("\nErrore nella scrittura di un log:");
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
	if ((LOG_FD = open("/usr/log/log.txt", O_RDWR | O_CREAT, 00700)) < 0){
		perror("\nErrore apertura file di log");
}
	
	
	
	
	logMessage("\tMessaggio 1 :\n");
	logMessage("\tMessaggio 2 :\n");
	logMessage("\tMessaggio 3 :\n");
	
	
	close(LOG_FD);
}
