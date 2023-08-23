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
TASK_ID DESTRUCTOR_TID;
TASK_ID DIAGNOSTICS_TID;

SEM_ID WIFI_CONTROL_SEM;                //semaforo per gestire l'accesso alle variabili globali condivise da wifiTask e controlTask
SEM_ID WIFI_DIAG_SEM;                   //semaforo per gestire l'accesso alle variabili globali condivise da wifiTask e diagTask
SEM_ID CONTROL_DIAG_SEM;                //semaforo per gestire l'accesso alle variabili globali condivise da constrolTask e diagTask
//____________________________________________________________________________________________________________________________
extern int CURRENT_HOST;          //host corrente che ha avviato il two-phase-commit-protocol

//____________________________________________________________________________________________________________________________

MSG_Q_ID IN_CONTROL_QUEUE;        // coda di messaggi da task_wifi->task_controllo
MSG_Q_ID OUT_CONTROL_QUEUE;       // coda di messaggi da task_controllo->task_wifi

#define MAX_CONN      50          //numero massimo di connessioni per un nodo
#define TAIL_ID      -9999        //Id associato al nodo successivo all'ultimo nodo di una route
#define MAX_LOG_SIZE  1024
#define MAX_LOG_BUFF  10
#define MAX_CTRL_SIZE 1024      
#define MAX_CTRL_BUFF 10
#define TYPE_SWITCH   2
#define TYPE_LINEAR   3 
#define LOG_LEVEL 0               // Verranno loggati tutti i logMessage con logLevel >= a LOG_LEVEL 
#define LOG_ACTIVE 1              // Stato che indica che si sta eseguendo il log
#define LOG_SUSPENDED 0           // Stato che indica che il log è momentaneamente sospeso
// TODO: controllare affidabilità della conversione tra tick e secondi effettivi  
#define TICKS_TO_SECOND 60        // Testando il codice si ha che 60 tick sono equivalenti ad 1 sec
#define FAIL FALSE                // FAIL è lo stato in cui la procedura di ping non è andata a buon fine
#define SUCCESS TRUE              // SUCCESS è lo stato in cui la procedura di ping è andata a buon fine
#define ROUTE_ID_PING 8888        //  Const int used in diagnostics task
#define HOST_ID_PING 7777         //  Const int used in diagnostics task
#define DIAG_TIME 10              // tempo (in secondi) da quando non si ricevono msg prima di avviare il task di diagnostica
#define SIM_SENSOR 1              // Se 1 indica che i SENSOR_ON sono simulati con messaggi, se 0 si utilizzano i pulsanti fisici



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

typedef struct{
    int last_route_id;  // indica l'ultima rotta per cui è stato richiesto il positioning
    bool in_position;   // indica l'esito del positioning
}railroad_switch;       // Questa struttura serve per simulare dove è posizionato il deviatoio

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
extern railroad_switch railswitch; // Questa variabile indica 

enum DIAG_STATUS{
    NOT_ACTIVE,
    STARTING,
    ACTIVE,
    ENDING
}ping_status; // Indica se lo stato della procedura di ping

enum PRI_LEV{
    PRI_0 = 61,
    PRI_1 = 60,
    PRI_2 = 59,
    PRI_3 = 58,
    PRI_4 = 57,
    PRI_5 = 56
}priority_levels; // Indica i livelli di priority, in VxWorks 0 è priorità massima e 255 minima

typedef struct{
    bool R;
    bool G;
    bool B;
}color;

typedef enum STATUS_COLORS{
    OFF_COL,
    INIT_COL,
    NOT_RESERVED_COL,
    MESSAGE_EXCHANGE_COL,
    POSITIONING_COL,
    RESERVED_COL,
    TRAIN_IN_TRANSITION_COL,
    FAIL_COL
}status_color;

extern bool ping_success;
extern int ping_answers;
extern bool diag_success;
extern bool diag_ended;
extern bool sensor_on_detected;

#endif /* INCLUDES_GLOBAL_VARIABLES_H_ */
