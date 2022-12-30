//Task di log
//Gestisce la scrittura su un file di log tramite una funzione utilizzabile dai vari task

#include "logTask.h"

int LOG_FD;


void mytest(void){
	printf("\nprova\n");
}

void logMessage(char* msg, char* task_name){
	char final_msg[1024] = "task : ";
	
	//TODO inserire controllo su lunghezza messaggio
	
	strcat(final_msg, task_name);
	strcat(final_msg, "\t");	
	strcat(final_msg, msg);	
	strcat(final_msg, "\n");
	if(write(LOG_FD, final_msg, strlen(final_msg)) < 0){
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
	
	
	logMessage("Messaggio log di prova", "tLog");
	
	
	close(LOG_FD);
	
}
