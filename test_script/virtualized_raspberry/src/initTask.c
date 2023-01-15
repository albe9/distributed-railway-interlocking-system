/*
 * initTask.c
 *
 *  Created on: Jan 3, 2023
 *      Author: alber
 */


#include "initTask.h"


void setCurrentTime(time_t current_time){
	
	struct timeval current_timeval;
	current_timeval.tv_sec = current_time;
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

void initMain(void){
	
	//apro la connessione con l'host per ricevere i dati di configurazione
	int HOST_PORT = 6543;
	// char HOST_IP[] = "192.168.1.35";
	char HOST_IP[] = "172.23.78.253";
	conn conn_host = {.fd = 0, .sock = 0};
	

	connectToServer(&conn_host, HOST_IP, HOST_PORT);
	char msg[50];
	snprintf(msg, 50, "Connessione da raspberry con RASP_ID : %i", RASP_ID);
	sendToServer(&conn_host, msg);
	char config_string[1024] = {0};
	
	readFromServer(&conn_host, config_string, 1024);
	printf("\n%s",config_string);
	

}

