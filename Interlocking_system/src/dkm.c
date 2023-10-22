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
	LOG_TID = taskCreate("LogTask", PRI_0, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
    taskCpuAffinitySet(LOG_TID, 1 << 2);
    taskActivate(LOG_TID);
}

// Eseguita in fase di reload da workbench.sh
void startDestructor(void){
	DESTRUCTOR_TID = taskSpawn("destructorTask", PRI_0, 0, 20000,(FUNCPTR) destructorMain, 0,0,0,0,0,0,0,0,0,0);
}


// Eseguita da launch_nodes.sh
void startInit(int rasp_id, char* host_ip, bool sim_sensor){
	// Setto l'id e l'ip del raspberry
	RASP_ID = rasp_id;
	snprintf(RASP_IP, 20, "192.168.1.21%i", RASP_ID);
	// Setto l'ip dell'host
	snprintf(HOST_IP, 20, host_ip);
	// Setto la modalità di lettura dei sensorOff
	SIM_SENSOR = sim_sensor;

	TIMER_TID = taskSpawn("timerTask", PRI_0, 0, 20000,(FUNCPTR) timerMain, 0,0,0,0,0,0,0,0,0,0);
	INIT_TID = taskSpawn("initTask", PRI_3, 0, 20000,(FUNCPTR) initMain, 0,0,0,0,0,0,0,0,0,0);
}

void test_gen_log(){

	struct stat st = {0};
	//Controlla se esiste directory di log, altrimenti la crea
	if (stat("/usr/log", &st) == -1) {
	    if(mkdir("/usr/log", 00700) < 0){
	    	perror("\nErrore creazione directory di log:");
	    }
	}

	if(remove("/usr/log/log.txt") == -1){
		perror("\nErrore nel resettare il file di log:");
	}

	int test_fd;
	if ((test_fd = open("/usr/log/log.txt",O_RDWR | O_APPEND  | O_CREAT, 00700)) < 0){
		perror("\nErrore apertura file di log");
	}

	char log_buffer[] = "TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST TEST\n"; 

	for(int i = 0;i < 1000; i++){
		if(write(test_fd, log_buffer, strlen(log_buffer)) == ERROR){
			perror("\nErrore nella scrittura di un log:");
		}
	}
	
	close(test_fd);
}

void test_log(){
	test_gen_log();
	start_log();
	taskDelay(2);
	sendLogToHost();

}