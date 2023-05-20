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
#include <stdbool.h>
#include <socket.h>


TASK_ID LOG_TID;
TASK_ID TEST_TID;
TASK_ID INIT_TID;
TASK_ID WIFI_TID;
TASK_ID CONTROL_TID;
TASK_ID POSITIONING_TID;
TASK_ID DIAGNOSTICS_TID;

SEM_ID GLOBAL_SEM;                //semaforo per gestire l'accesso alle variabili globali condivise da più task
//____________________________________________________________________________________________________________________________
extern int CURRENT_HOST;          //host corrente che ha avviato il two-phase-commit-protocol

//____________________________________________________________________________________________________________________________

MSG_Q_ID IN_CONTROL_QUEUE;        // coda di messaggi da task_wifi->task_controllo
MSG_Q_ID OUT_CONTROL_QUEUE;       // coda di messaggi da task_controllo->task_wifi
MSG_Q_ID IN_DIAGNOSTICS_QUEUE;    // coda di messaggi da task_wifi->task_diagnostica  
MSG_Q_ID OUT_DIAGNOSTICS_QUEUE;   // coda di messaggi da task_diagnostica->task_wifi

#define MAX_CONN      50          //numero massimo di connessioni per un nodo
#define TAIL_ID      -9999        //Id associato al nodo successivo all'ultimo nodo di una route
#define MAX_LOG_SIZE  1024
#define MAX_LOG_BUFF  10
#define MAX_CTRL_SIZE 1024      
#define MAX_CTRL_BUFF 10
#define TYPE_SWITCH   2
#define TYPE_LINEAR   3  
#define LOG_ACTIVE 1              // Stato che indica che si sta eseguendo il log
#define LOG_SUSPENDED 0           // Stato che indica che il log è momentaneamente sospeso
#define TICKS_TO_SECOND 60        // Testando il codice si ha che 60 tick sono equivalenti ad 1 sec

typedef struct{
    int route_id;
    int rasp_id_prev;
    int rasp_id_next;
}route;

typedef struct{
    char command[50];
    int sender_id;
    int recevier_id;
    int route_id;
    int host_id;
}tpcp_msg;

extern int SERVER_PORT;
extern int LOG_PORT;
extern int RASP_ID;
extern char HOST_IP[20];
extern char RASP_IP[20];
extern int log_status;
extern route *node_routes;
extern int route_count;
extern int NODE_TYPE;
extern bool IN_POSITION;

#endif /* INCLUDES_GLOBAL_VARIABLES_H_ */
