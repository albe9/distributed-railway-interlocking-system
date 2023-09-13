/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <errno.h>
#include <taskLib.h>
#include <clockLib.h>
#include <sysLib.h>

#include "global_variables.h"
#include "initTask.h"
#include "wifiTask.h"
#include "destructorTask.h"
#include "timerTask.h"
#include "gpio.h"

void test_size(void){
	printf("%i", sizeof(u_int64_t));
}

void start_timer(void){
	TIMER_TID = taskSpawn("timerTask", PRI_0, 0, 20000,(FUNCPTR) timerMain, 0,0,0,0,0,0,0,0,0,0);
}

void start_log(void){
	LOG_TID = taskSpawn("LogTask", PRI_0, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
}


void test_timer1(void)
{
	UINT64 log_time_micro = 0;
	semTake(TIMER_SEM, WAIT_FOREVER);
	log_time_micro = totalCurrentTimeMicro;
	semGive(TIMER_SEM);
	// sommo al tempo totale dato dagli overflow il tempo attuale in microsecondi
	log_time_micro += (sysTimestamp())/54;

	char log_micro[100];
	snprintf(log_micro, 100, "%llu    ", log_time_micro);
	printf("%s\n", log_micro);
}
test_timer2(void){
	u_int64_t time_micro = 0;
	time_micro = getTimeMicro();
	printf("%llu\n", time_micro);
}

void test_timer(void){
	test_timer1();
	test_timer2();
}

void startDestructor(void){
	DESTRUCTOR_TID = taskSpawn("destructorTask", PRI_0, 0, 20000,(FUNCPTR) destructorMain, 0,0,0,0,0,0,0,0,0,0);
}

void startInit(int rasp_id, char* host_ip){
	//Setto l'id e l'ip del raspberry
	RASP_ID = rasp_id;
	snprintf(RASP_IP, 20, "192.168.1.21%i", RASP_ID);
	//Setto l'ip dell'host
	snprintf(HOST_IP, 20, host_ip);
	printf("\n\n%s", HOST_IP);
	TIMER_TID = taskSpawn("timerTask", PRI_0, 0, 20000,(FUNCPTR) timerMain, 0,0,0,0,0,0,0,0,0,0);
	INIT_TID = taskSpawn("initTask", PRI_0, 0, 20000,(FUNCPTR) initMain, 0,0,0,0,0,0,0,0,0,0);
}

void testColor() {
	changeLedColor(OFF_COL); //spento
	sleep(6);
	changeLedColor(INIT_COL); //CIANO
	sleep(6);
	changeLedColor(NOT_RESERVED_COL); //BLU
	sleep(6);
	changeLedColor(MESSAGE_EXCHANGE_COL); //GIALLO
	sleep(6);
	changeLedColor(POSITIONING_COL); //BIANCO
	sleep(6);
	changeLedColor(RESERVED_COL); //VERDE
	sleep(6);
	changeLedColor(TRAIN_IN_TRANSITION_COL); //MAGENTA
	sleep(6);
	changeLedColor(FAIL_COL);
	sleep(6);
	changeLedColor(OFF_COL);
	while (true)
	{
		if (readButton()){
			changeLedColor(MESSAGE_EXCHANGE_COL);
			sleep(6);
			break;
		}
	}
	
}
