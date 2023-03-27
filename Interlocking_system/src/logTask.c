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
	snprintf(log_ms, 100, "%ims ", (int)(log_time.tv_nsec * 0.000001));

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

int getSizeofLog(char *path_to_file){
	// Apriamo il file
	FILE *file;
	if(file = fopen(path_to_file, "r") < 0){
		logMessage("Problema nell'apertura del file", taskName(0));
		return E_LOG_OPEN;
	}
	if (file != NULL) {
    	/* Go to the end of the file. */
		if (fseek(file, 0L, SEEK_END) == 0) {
			/* Get the size of the file. */
			int bufsize = ftell(file);
			if (bufsize == -1) {
				logMessage("File di dimensioni nulla", taskName(0));
				return E_LOG_EMPTY;
			}
			printf("bufsize %i", bufsize);
		fclose(file);
		return bufsize;
		}
	}
	else{
		logMessage("Puntatore nullo", taskName(0));
		return E_LOG_EMPTY;
	}
}

exit_number logToHost(void){
	// Si sospende il task che esegue il logInit
	if(taskSuspend(LOG_TID) < 0){
		return E_DEFAUL_ERROR;
	}
	// Si crea un socket verso l'host
	connection host_conn;
	connectToServer(&host_conn, HOST_IP, LOG_PORT);
	// Si legge quanto è lungo il file 
	int logSize = 0;
	if(logSize = getSizeofLog("/usr/log/log.txt") < 0){
		// E si copia il contenuto in un buffer
		char *logMsg = malloc(sizeof(char) * (logSize + 1));
		FILE *file = fopen("/usr/log/log.txt", "r");
		/* Go back to the start of the file. */
        if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }
        /* Read the entire file into memory. */
        size_t newLen = fread(logMsg, sizeof(char), logSize, file);
        if ( ferror(file) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            logMsg[newLen++] = '\0'; /* Just to be safe. */
        }
		prinf("%s", logMsg);
		fclose(file);
		// Si invia il messaggio
		sendToConn(&host_conn, logMsg);
		shutdown(host_conn.sock, SHUT_RDWR);
		if (close(host_conn.sock) < 0){
			return E_DEFAUL_ERROR;
		}
		return E_SUCCESS;
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






