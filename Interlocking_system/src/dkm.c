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


// Eseguibili da telnet
void start_timer(void){
	TIMER_TID = taskSpawn("timerTask", PRI_0, 0, 20000,(FUNCPTR) timerMain, 0,0,0,0,0,0,0,0,0,0);
}

void start_log(void){
	LOG_TID = taskSpawn("LogTask", PRI_0, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
}
// Eseguita in fase di reload da workbench.sh
void startDestructor(void){
	DESTRUCTOR_TID = taskSpawn("destructorTask", PRI_0, 0, 20000,(FUNCPTR) destructorMain, 0,0,0,0,0,0,0,0,0,0);
}


// Eseguita da launch_nodes.sh
void startInit(int rasp_id, char* host_ip){
	//Setto l'id e l'ip del raspberry
	RASP_ID = rasp_id;
	snprintf(RASP_IP, 20, "192.168.1.21%i", RASP_ID);
	//Setto l'ip dell'host
	snprintf(HOST_IP, 20, host_ip);

	TIMER_TID = taskSpawn("timerTask", PRI_0, 0, 20000,(FUNCPTR) timerMain, 0,0,0,0,0,0,0,0,0,0);
	INIT_TID = taskSpawn("initTask", PRI_3, 0, 20000,(FUNCPTR) initMain, 0,0,0,0,0,0,0,0,0,0);
}