//Task di log
//Gestisce la scrittura su un file di log tramite una funzione utilizzabile dai vari task

#include "logTask.h"



int LOG_FD;
MSG_Q_ID LOG_QUEUE;


void mytest(void){
	
	logMessage("Messaggio log di prova 1", taskName(LOG_TID));
	logMessage("Messaggio log di prova 2", taskName(LOG_TID));
	logMessage("Messaggio log di prova 3", taskName(LOG_TID));
}



//Funzione che scrive i messaggi sul file di log, gestisce l'accesso concorrente al file con un semaforo 

void logMessage(char* msg, char* task_name){
	
	char final_msg[MAX_LOG_SIZE] = "";
		
	//TODO inserire controllo su lunghezza messaggio
	
	
	//Salvo il timestamp
	time_t rawtime;
	struct tm * timeinfo;
	
	char timestamp[100];
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	strftime(timestamp,100,"(%F) %Hh:%Mm:%Ss:",timeinfo);

	//Aggiungo i secondi ed i millisecondi 
	struct timespec log_time;
	clock_gettime(CLOCK_REALTIME, &log_time);
	char log_ms[100];
	snprintf(log_ms, 100, "%03dms ", (int)(log_time.tv_nsec * 0.000001));

	//appendo timestamp e taskname
	strcat(final_msg, timestamp);
	strcat(final_msg, log_ms);
	strcat(final_msg, task_name);
	strcat(final_msg, "    ");	
	strcat(final_msg, msg);	
	strcat(final_msg, "\n");
	
	//salvo il messaggio sulla coda
	if(msgQSend(LOG_QUEUE, final_msg, strlen(final_msg), WAIT_FOREVER, MSG_PRI_NORMAL) == ERROR){
		perror("\nErrore durante l'inserimento nella coda dei log : ");
	}
	
	
}

void logInit(void){
	//Crea la directory e il file di log se non sono gi� esistenti
	
	struct stat st = {0};
	//Controlla se esiste directory di log, altrimenti la crea
	if (stat("/usr/log", &st) == -1) {
	    if(mkdir("/usr/log", 00700) < 0){
	    	perror("\nErrore creazione directory di log:");
	    }
	}
	//reset del file di log se esiste
	remove("/usr/log/log.txt");
	//Crea/apre il file di log in modalità append
	if ((LOG_FD = open("/usr/log/log.txt",O_RDWR | O_APPEND  | O_CREAT, 00700)) < 0){
		perror("\nErrore apertura file di log");
	}
	
	//creo la coda di messaggi per il log
	LOG_QUEUE = msgQCreate(MAX_LOG_BUFF, MAX_LOG_SIZE, MSG_Q_FIFO);
	
	
	
	char log_buffer[MAX_LOG_SIZE] = {0}; 
	//aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, logDestructor);
	//main loop del task, controlla la coda dei messaggi di log e li scrive su file
	while(true){
		//mi metto in attesa di un messaggio
		msgQReceive(LOG_QUEUE, log_buffer, MAX_LOG_SIZE, WAIT_FOREVER);
				
		//scrivo il messaggio presente in coda sul file di log
		if(write(LOG_FD, log_buffer, strlen(log_buffer)) == ERROR){
			perror("\nErrore nella scrittura di un log:");
			taskDelete(LOG_TID);
		}
		//resetto il buffer per accettare nuovi messaggi
		memset(log_buffer, 0, MAX_LOG_SIZE);			
	}
	
}	


void logDestructor(int sig){

	if(close(LOG_FD) < 0 ){
		perror("\nErrore durante la chiusura del LOG_FD");
	}
	taskDelete(0);

}



