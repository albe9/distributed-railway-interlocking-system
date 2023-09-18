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



void start_timer(void){
	TIMER_TID = taskSpawn("timerTask", PRI_0, 0, 20000,(FUNCPTR) timerMain, 0,0,0,0,0,0,0,0,0,0);
}

void start_log(void){
	LOG_TID = taskSpawn("LogTask", PRI_0, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
}



void test_core1(void){
	printf("task1 start\n");
	while(true){
		int a = 0;
		for(int i = 0; i< 100000000; i++){
			a++;
		}
		break;
	}
	printf("task1 end\n");
}

void test_core2(void){
	printf("task2 start\n");
	while(true){
		int a = 0;
		for(int i = 0; i< 100000000; i++){
			a++;
		}
		break;
	}
	printf("task2 end\n");
}

test_core(void){
	TASK_ID core1, core2;
	core1 = taskCreate("testCore1", PRI_1, 0, 20000,(FUNCPTR) test_core1, 0,0,0,0,0,0,0,0,0,0);
	taskCpuAffinitySet(core1, 1 << 3);
	core2 = taskCreate("testCore2", PRI_0, 0, 20000,(FUNCPTR) test_core2, 0,0,0,0,0,0,0,0,0,0);
	taskCpuAffinitySet(core2, 1 << 3);

	taskActivate(core1);
	taskActivate(core2);
}

void test_delay_timer(void){
	uint64_t previus_micro =0, current_micro = 0;
	uint64_t elapsed[100];

	for(int i = 0; i< 100; i++){
		current_micro = getTimeMicro();
		// logMessage("prova per il delay della funzione logMessage","dmkTest", 1);
		taskDelay(1);
		elapsed[i] = current_micro - previus_micro;
		previus_micro = current_micro;
	}

	for(int i = 0; i< 100; i++){
		printf("%llu\n", elapsed[i]); 
	}
}

void test_timer1(void)
{
	uint64_t log_time_micro = 0;
	semTake(TIMER_SEM, WAIT_FOREVER);
	log_time_micro = totalCurrentTimeMicro;
	semGive(TIMER_SEM);
	printf("Timer task ha riportato : %llu\n", log_time_micro);
	// sommo al tempo totale dato dagli overflow il tempo attuale in microsecondi
	log_time_micro += (sysTimestamp())/54;
	printf("%llu\n", log_time_micro);
	// char log_micro[100];
	// snprintf(log_micro, 100, "%llu    ", log_time_micro);
	// printf("%s\n", log_micro);
}
void test_timer2(void){
	uint64_t time_micro = 0;
	time_micro = getTimeMicro();
	printf("%llu\n", time_micro);
}

void test_timer(void){
	test_timer1();
	test_timer2();
	printf("\n");

	// UINT64	time_nano = 0;
	// sysTimestamp64(&time_nano);
	// printf("%llu\n", time_nano);
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
