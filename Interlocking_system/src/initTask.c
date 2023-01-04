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
	
	
	setCurrentTime(1672855095);
	
	
	
}

