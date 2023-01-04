//Task di log
//Gestisce la scrittura su un file di log tramite una funzione utilizzabile dai vari task

#include "logTask.h"

#define MAX_LOG_SIZE 1024
#define MAX_LOG_BUFF 3

SEM_ID SEM_LOG_FILE;
SEM_ID SEM_LOG_MSG;
int LOG_FD;
BOOL FLAG_LOG_ACTIVE = FALSE;

struct string_queue{
	char buffer[MAX_LOG_BUFF][MAX_LOG_SIZE];
	int log_number;
};
struct string_queue log_buffer;


void mytest(void){
	
	logMessage("Messaggio log di prova 1", taskName(LOG_TID));
	logMessage("Messaggio log di prova 2", taskName(LOG_TID));
	logMessage("Messaggio log di prova 3", taskName(LOG_TID));
}



//Funzione che scrive i messaggi sul file di log, gestisce l'accesso concorrente al file con un semaforo 

void logMessage(char* msg, char* task_name){
	char final_msg[MAX_LOG_SIZE] = "task : ";
	
	//TODO inserire controllo su lunghezza messaggio
	
	
	//Salvo il timestamp
	time_t rawtime;
	struct tm * timeinfo;
	
	char timestamp[80];
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	strftime(timestamp,80,"%H:%M (%F)%t",timeinfo);
	
	strcpy(final_msg, timestamp);
	strcat(final_msg, task_name);
	strcat(final_msg, "\t");	
	strcat(final_msg, msg);	
	strcat(final_msg, "\n");
	
	//acquisisco il semaforo e scrivo sul buffer
	if(semTake(SEM_LOG_FILE, WAIT_FOREVER) == ERROR){
		perror("\nErrore nell'acquisire semaforo del file di log");
		return;
	}
	
	if(FLAG_LOG_ACTIVE == FALSE){
		perror("Log task non attivo");
		return;
	}
	
	//controllo se c'è spazio e aggiungo il messaggio alla coda
	if(log_buffer.log_number < MAX_LOG_BUFF){
		strcpy(log_buffer.buffer[log_buffer.log_number], final_msg);
		log_buffer.log_number++;
		
		semGive(SEM_LOG_MSG);
	}
	
	//rilascio il semaforo
	if(semGive(SEM_LOG_FILE) == ERROR){
		perror("\nErrore nel rilasciare il semaforo del file di log");
		return;
	}
	
	semGive(SEM_LOG_MSG);
	
}



void logInit(void){
	//Crea la directory e il file di log se non sono già esistenti
	
	struct stat st = {0};
	//Controlla se esiste directory di log, altrimenti la crea
	if (stat("/usr/log", &st) == -1) {
	    if(mkdir("/usr/log", 00700) < 0){
	    	perror("\nErrore creazione directory di log:");
	    }
	}
	//Crea/apre il file di log in modalità append
	if ((LOG_FD = open("/usr/log/log.txt",O_RDWR | O_APPEND  | O_CREAT, 00700)) < 0){
		perror("\nErrore apertura file di log");
	}
	
	//creo il semaforo per regolare l'accesso al file e per notificare l'arrivo di msg di log
	SEM_LOG_FILE = semBCreate(SEM_Q_FIFO, SEM_FULL);
	SEM_LOG_MSG = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
	
	
	//inizializzo la coda per il buffer e abilito la funzione logMessage(tramite FLAG_LOG_ACTIVE)
	log_buffer.log_number = 0;
	FLAG_LOG_ACTIVE = TRUE;
	
	//main loop del task, controlla la coda dei messaggi di log e li scrive su file
	while(TRUE){
		
		//mi metto in attesa di un messaggio
		if(semTake(SEM_LOG_MSG, WAIT_FOREVER) == ERROR){
			perror("\nErrore nell'acquisire semaforo dei messaggi di log");
			taskDelete(LOG_TID);
		}
		//acquisisco il semaforo e scrivo sul file
		if(semTake(SEM_LOG_FILE, WAIT_FOREVER) == ERROR){
			perror("\nErrore nell'acquisire semaforo del file di log");
			taskDelete(LOG_TID);
		}
		
		//scrivo tutti i messaggi presenti in coda sul file di log
		
		
		for(int log_idx = 0;log_idx < log_buffer.log_number;log_idx++){
			if(write(LOG_FD, log_buffer.buffer[log_idx], strlen(log_buffer.buffer[log_idx])) < 0){
				perror("\nErrore nella scrittura di un log:");
				taskDelete(LOG_TID);
			}
			
		}
		log_buffer.log_number = 0;
			
		
		//rilascio il semaforo
		if(semGive(SEM_LOG_FILE) == ERROR){
			perror("\nErrore nel rilasciare il semaforo del file log");
			taskDelete(LOG_TID);
		}
	}
	
}	






