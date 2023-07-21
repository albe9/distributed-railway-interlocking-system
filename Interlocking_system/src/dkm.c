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
#include <gpioLib.h>


TASK_ID taskId;


//test per il passaggio di stringhe come parametro
/*
char message[20] = "log di prova";

void myRoutine(_Vx_usr_arg_t message){
	logMsg("Messaggio di prova : %s", message, 0, 0, 0, 0, 0);
	logMsg("Task name : %s", taskName(0), 0, 0, 0, 0, 0);
}


//test per creare un elenco di task con relativi id (per disattivarli ecc) 
void myRoutine4(void){
	int maxTasks = 50;
	TASK_ID id_list[maxTasks];
	TASK_DESC task_info;

	
	taskIdListGet(id_list, maxTasks);
	for(int i=0;i<maxTasks;i++)
	{
		if(taskInfoGet(id_list[i], &task_info) == ERROR){
			printf("\nErrore al task numero : %i\n",i);
		}
		else{
			printf("\ntask : %s\n",task_info.td_name);
		}
	}
	
}

void start3(void){
	taskId = taskSpawn("test task", 1, 0, 20000,(FUNCPTR) myRoutine4, 0,0,0,0,0,0,0,0,0,0);
}

*/
void startDestructor(void){
	DESTRUCTOR_TID = taskSpawn("destructorTask", PRI_0, 0, 20000,(FUNCPTR) destructorMain, 0,0,0,0,0,0,0,0,0,0);
}

void startLog(void){
	LOG_TID = taskSpawn("logTask", 100, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
}

void test(void){
	clock_t timer1, timer2;
    double elapsed;

    timer1 = tickGet();
	taskDelay(TICKS_TO_SECOND * 3);
	timer2 = tickGet();
    elapsed = (double)(timer2 - timer1)/TICKS_TO_SECOND;
	printf("\n%li\n%li\n%f\n", timer1, timer2, elapsed);
}

void startInit(int rasp_id, char* host_ip){
	//Setto l'id e l'ip del raspberry
	RASP_ID = rasp_id;
	snprintf(RASP_IP, 20, "192.168.1.21%i", RASP_ID);
	//Setto l'ip dell'host
	snprintf(HOST_IP, 20, host_ip);
	printf("\n\n%s", HOST_IP);
	INIT_TID = taskSpawn("initTask", PRI_0, 0, 20000,(FUNCPTR) initMain, 0,0,0,0,0,0,0,0,0,0);
}

void startWifi(void){
	WIFI_TID = taskSpawn("wifiTask", 50, 0, 20000,(FUNCPTR) wifiMain, 0,0,0,0,0,0,0,0,0,0);
}

// void reset(void){
// 	resetConnections();
// }

void blinkLed(){
	pinMode(LED_RED, OUT);
	pinMode(LED_GREEN, OUT);
	pinMode(LED_BLU, OUT);
	sleep(3);
	gpioWrite(LED_RED, HIGH);
	sleep(1);
	gpioWrite(LED_RED, LOW);
	gpioWrite(LED_GREEN, HIGH); 
	sleep(1); 
	gpioWrite(LED_GREEN, LOW);
	gpioWrite(LED_BLU, HIGH);
	sleep(1);
	gpioWrite(LED_BLU, LOW);

	gpioFree(LED_RED);
	gpioFree(LED_GREEN);
	gpioFree(LED_BLU);
	sleep(1);
}

void readButton() {
    int stato = 0;
	int i = 0;

	pinMode(LED_RED, OUT);
	pinMode(LED_GREEN, OUT);

	pinMode(BUTTON, IN);

	while (i < 100)
	{
		stato = gpioRead(BUTTON);
		printf("STATO: %d \n", stato);
		if(stato == 1){
			gpioWrite(LED_RED, LOW);
			gpioWrite(LED_GREEN, HIGH);
		}
		else {
			gpioWrite(LED_GREEN, LOW);
			gpioWrite(LED_RED, HIGH);
		}
		sleep(1);
		i++;
	}
	gpioFree(LED_RED);
	gpioFree(LED_GREEN);
}
