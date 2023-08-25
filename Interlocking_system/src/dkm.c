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
#include "gpio.h"


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
