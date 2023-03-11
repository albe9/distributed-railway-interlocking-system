/*
 * global_variables.h
 *
 *  Created on: Jan 2, 2023
 *      Author: alber
 *      
 *      
 *    Variabili globali (es ID task e semafori) e headers comuni tra tutti i task 
 */



#ifndef INCLUDES_GLOBAL_VARIABLES_H_
#define INCLUDES_GLOBAL_VARIABLES_H_

#include "errors.h"
#include <errno.h>
#include <fcntl.h>
#include <msgQLib.h>
#include <semLib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <taskHookLib.h>
#include <taskLib.h>
#include <time.h>
#include <unistd.h>


TASK_ID LOG_TID;
TASK_ID TEST_TID;
TASK_ID INIT_TID;
TASK_ID WIFI_TID;
TASK_ID CONTROL_TID;

SEM_ID GLOBAL_SEM;                //semaforo per gestire l'accesso alle variabili globali condivise da più task

MSG_Q_ID CONTROL_QUEUE;

#define MAX_CONN 50               //numero massimo di connessioni per un nodo
#define TAIL_ID -9999             //Id associato al nodo successivo all'ultimo nodo di una route
#define MAX_LOG_SIZE 1024
#define MAX_LOG_BUFF 10
#define MAX_CTRL_SIZE 1024      
#define MAX_CTRL_BUFF 10


extern int CURRENT_HOST;          //host corrente che ha avviato il two-phase-commit-protocol
extern int SERVER_PORT;
extern int RASP_ID;
extern char HOST_IP[20];
extern char RASP_IP[20];


#endif /* INCLUDES_GLOBAL_VARIABLES_H_ */
