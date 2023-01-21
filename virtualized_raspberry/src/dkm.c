/* includes */

#include <vxWorks.h>
#include <stdio.h>
#include <errno.h>
#include <taskLib.h>


#include "global_variables.h"
#include "initTask.h"
#include "logTask.h"
#include "wifiTask.h"


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



void startLog(void){
	LOG_TID = taskSpawn("LogTask", 100, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
}

void test(void){
	TEST_TID = taskSpawn("tTest", 50, 0, 20000,(FUNCPTR) mytest, 0,0,0,0,0,0,0,0,0,0);
}

void startInit(void){
	INIT_TID = taskSpawn("initTask", 50, 0, 20000,(FUNCPTR) initMain, 0,0,0,0,0,0,0,0,0,0);
}

void startWifi(void){
	WIFI_TID = taskSpawn("wifi_task", 50, 0, 20000,(FUNCPTR) wifiMain, 0,0,0,0,0,0,0,0,0,0);
}


