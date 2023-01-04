/*
 * initTask.c
 *
 *  Created on: Jan 3, 2023
 *      Author: alber
 */


#include "initTask.h"


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
