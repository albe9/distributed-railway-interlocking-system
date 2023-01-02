//Task di log
//Gestisce la scrittura su un file di log tramite una funzione utilizzabile dai vari task

#include "logTask.h"
#include <sys/time.h>


int LOG_FD;


void mytest(void){
	
	

}

void setCurrentTime(void){
	struct tm current_time;
	current_time.tm_sec  = 0;				//seconds after the minute – [0, 61] (until C99)[0, 60]
	current_time.tm_min  = 37;				//minutes after the hour – [0, 59]
	current_time.tm_hour = 20;				//hours since midnight – [0, 23]
	current_time.tm_mday = 2;				//day of the month – [1, 31]
	current_time.tm_mon  = 1 - 1;			//months since January – [0, 11]
	current_time.tm_year = 2023 - 1900;		//years since 1900
	current_time.tm_wday = 0;				//days since Sunday – [0, 6]
	current_time.tm_yday = 1;				//days since January 1 – [0, 365]
	

	time_t current_time_clike = mktime(&current_time);
	struct timeval current_timeval;
	current_timeval.tv_sec = current_time_clike;
	current_timeval.tv_usec = 0;
	
	
	if(settimeofday(&current_timeval, NULL) < 0){
		perror("Errore nel settare il tempo :");
	}
	
	
	time_t rawtime;
	struct tm * timeinfo;
	
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	printf ("Current local time and date: %s", asctime(timeinfo));	
}


//Funzione che scrive i messaggi sul file di log, gestisce l'accesso concorrente al file con un semaforo 

void logMessage(char* msg, char* task_name){
	char final_msg[1024] = "task : ";
	
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
	
	logMessage("Messaggio log di prova", taskName(LOG_TID));
	
	
	//test
	close(LOG_FD);
	
}
