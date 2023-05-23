/*
 * initTask.c
 *
 *  Created on: Jan 3, 2023
 *      Author: alber
 */


#include "initTask.h"
#define MAX_LOG_SIZE 1024
#define MAX_LOG_BUFF 10

void setCurrentTime(time_t current_time){
	
	struct timeval current_timeval;
	current_timeval.tv_sec = current_time;
	current_timeval.tv_usec = 0;
	
	
	if(settimeofday(&current_timeval, NULL) < 0){
		perror("Errore nel settare il tempo :");
	}
	
	
	time_t rawtime;
	struct tm * timeinfo;
	
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	printf ("Current local time and date: %s", asctime(timeinfo));	
}

static network node_net;
static time_t current_time = 0;


exit_number parseConfigString(char* config_string,route **routes, network *net){

    char *packet[4];

    //tokenizzo e salvo tutte le tipologie di pacchetti

    for(int packet_idx = 0; packet_idx < 4; packet_idx++){
        if(packet_idx == 0){
            packet[packet_idx] = strtok(config_string, ";");
        }
        else {
            packet[packet_idx] = strtok(NULL, ";");
        }
        if(packet[packet_idx] == NULL)return(E_PARSING);
    }
    
    //analizzo il primo pacchetto
    
    if(sscanf(packet[0],"%li,%i,%i,%i", &current_time, &net->prev_node_count, &net->next_node_count, &net->route_count) != 4)return E_PARSING;

    //alloco lo spazio per le stringhe degli ip precedenti e successivi e per le routes
    net->prev_ips = malloc(net->prev_node_count * sizeof(char *));
    for(int ip_idx = 0; ip_idx < net->prev_node_count; ip_idx++){
        net->prev_ips[ip_idx] = malloc(IP_LEN * sizeof(char));
    }

    net->next_ips = malloc(net->next_node_count * sizeof(char *));
    for(int ip_idx = 0; ip_idx < net->next_node_count; ip_idx++){
        net->next_ips[ip_idx] = malloc(IP_LEN * sizeof(char));
    }

    net->prev_ids = malloc(net->prev_node_count * sizeof(int));
    net->next_ids = malloc(net->next_node_count * sizeof(int));


    *routes = (route*)malloc(net->route_count * sizeof(route));
    route_count = net->route_count;

    //secondo pacchetto
    char *ip;
    
    for(int ip_idx = 0; ip_idx < net->prev_node_count; ip_idx++){
        if (ip_idx == 0){
            ip = strtok(packet[1], ",");
        }
        else{
            ip = strtok(NULL, ",");
        }
        if(ip == NULL)return(E_PARSING);

        strcpy(net->prev_ips[ip_idx],ip);
    }
    char* id;

    for(int id_idx = 0; id_idx < net->prev_node_count; id_idx++){
        id = strtok(NULL, ",");
        if(id == NULL)return(E_PARSING);
        else{
            net->prev_ids[id_idx] = atoi(id);
        }

    }

    //terzo pacchetto
    
    for(int ip_idx = 0; ip_idx < net->next_node_count; ip_idx++){
        if (ip_idx == 0){
            ip = strtok(packet[2], ",");
        }
        else{
            ip = strtok(NULL, ",");
        }
        if(ip == NULL)return(E_PARSING);

        strcpy(net->next_ips[ip_idx],ip);
    }

    for(int id_idx = 0; id_idx < net->next_node_count; id_idx++){
        id = strtok(NULL, ",");
        if(id == NULL)return(E_PARSING);
        else{
            net->next_ids[id_idx] = atoi(id);
        }

    }

    //quarto pacchetto
    char *route_data;
    for(int route_idx = 0; route_idx < net->route_count; route_idx++){
        if (route_idx == 0){
            route_data = strtok(packet[3], "/");
        }
        else{
            route_data = strtok(NULL, "/");
        }
        if(route_data == NULL)return(E_PARSING);


        if(sscanf(route_data,"%i,%i,%i", &(*routes)[route_idx].route_id,
                                &(*routes)[route_idx].rasp_id_prev, &(*routes)[route_idx].rasp_id_next) != 3)return E_PARSING;
    }


    return(E_SUCCESS);
}

void printConfigInfo(route *routes, network *net){
	printf("Config del nodo:\n\t-Nodi precedenti : %i\n\t-Nodi successivi : %i\n\t-Routes : %i\n",
			net->prev_node_count,net->next_node_count,net->route_count);
	printf("Ip precendenti (id) : \n");
	for(int node_idx = 0;node_idx < net->prev_node_count;node_idx ++){
		printf("\t-%s (%i)\n",net->prev_ips[node_idx], net->prev_ids[node_idx]);
	}
	printf("Ip successivi (id) : \n");
	for(int node_idx = 0;node_idx < net->next_node_count;node_idx ++){
		printf("\t-%s (%i)\n",net->next_ips[node_idx], net->next_ids[node_idx]);
	}
	printf("Routes : \n");
	for(int route_idx = 0;route_idx < net->route_count;route_idx++){
		printf("\t-Route id : %i\n\t-Node id precedente : %i\n\t-Node id successivo : %i\n\n",
				routes[route_idx].route_id, routes[route_idx].rasp_id_prev, routes[route_idx].rasp_id_next);
	}
	printf("--------------------------------------------------------\n");
}

void initMain(void){
	
    

    // starto il logTask
    LOG_TID = taskSpawn("LogTask", 100, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);

	//apro la connessione con l'host per ricevere i dati di configurazione
	
	connection host_s = {.sock=0, .connected_id=0};
	

	connectToServer(&host_s, HOST_IP, SERVER_PORT);
	char msg[100];
	snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
	sendToConn(&host_s, msg);
	char config_string[1024] = {0};
	
	readFromConn(&host_s, config_string, 1024);
    exit_number parsing_status;
	if((parsing_status = parseConfigString(config_string, &node_routes, &node_net)) != E_SUCCESS){
        logMessage(errorDescription(parsing_status), taskName(0));
        exit(-1);
	}

    setCurrentTime(current_time);

    logMessage("Configurazione ricevuta", taskName(0));
    //printConfigInfo(node_routes, &node_net);
    
    


    //Notifico l'host dell'avvenuta configurazione
    memset(msg, 0, 100);
    snprintf(msg, 100, "Configurazione eseguita su RASP_ID : %i", RASP_ID);
    sendToConn(&host_s, msg);
    
    //Prima di procedere attendo che l'host mi notifichi l'avvenuta configurazione di tutti i nodi
    memset(msg, 0, 100);
    readFromConn(&host_s, msg, 100);
    logMessage(msg, taskName(0));
	
    //chiudo la connessione con l'host
    shutdown(host_s.sock, SHUT_RDWR);
		if (close(host_s.sock) < 0){
			logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
		}

    //qui parte il protocollo per instaurare le connessioni dei nodi a catena
    //prima fase : client
    
    
    for(int node_idx=0; node_idx<node_net.prev_node_count; node_idx++){
        //tento di connettermi al nodo precedente ripetutamente finchè non apre la connessione
        exit_number conn_status = 0;
        do{
            conn_status = addConnToServer(node_net.prev_ips[node_idx], SERVER_PORT, node_net.prev_ids[node_idx]);
            if(conn_status != E_CONN_REFUSED && conn_status != E_SUCCESS){
                logMessage(errorDescription(conn_status), taskName(0));
            }
        }while(conn_status == E_CONN_REFUSED);
    }
    

    //seconda fase : server

    //abbiamo assunto che nessuna route può terminare con uno scambio,
    //quindi tutti i nodi di terminazione avranno un solo nodo successivo con id : TAIL_ID

    if(node_net.next_node_count == 1 && node_net.next_ids[0] == TAIL_ID){
        // Caso nodo di terminazione
        memset(msg, 0, 100);
        snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
        for(int node_idx=0; node_idx<node_net.prev_node_count; node_idx++){
            sendToConn(getConnByIndex(node_idx), msg);
        }
    }
    else{
        exit_number conn_status = 0;
        if((conn_status = addConnToClient(node_net.next_node_count)) != E_SUCCESS){
            logMessage(errorDescription(conn_status), taskName(0));
        }
        // Attendo che tutti i nodi collegati notifichino l'avvenuta connessione
        for(int node_idx=0; node_idx<node_net.next_node_count; node_idx++){
            memset(msg, 0, 100);
            readFromConn(getConnByIndex(node_net.prev_node_count + node_idx), msg, 100);
            logMessage(strncat(msg, " ha stabilito tutte le connessioni", 100), taskName(0));
        }
        // Ricevuta la notifica da tutt i nodi successivi , informo quelli precedenti
        memset(msg, 0, 100);
        snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
        for(int node_idx=0; node_idx<node_net.prev_node_count; node_idx++){
            sendToConn(getConnByIndex(node_idx), msg);
        }
    }


    //TODO eliminare strutture e memoria allocata per il taskInit una volta concluso

    //Setto il raspberry come nodo di scambio o lineare in base alla config ricevuta
    if(node_net.next_node_count > 1 || node_net.prev_node_count > 1){
        NODE_TYPE = TYPE_SWITCH;
    }
    else{
        NODE_TYPE = TYPE_LINEAR;
    }

    logMessage("InitTask completato", taskName(0));
    // debug
    // memset(msg, 0, 100);
	// snprintf(msg, 100, "nodo di tipo : %i", NODE_TYPE);
	// logMessage(msg,taskName(0));


    GLOBAL_SEM = semBCreate(SEM_Q_FIFO, SEM_FULL);
    IN_CONTROL_QUEUE = msgQCreate(MAX_LOG_BUFF, MAX_LOG_SIZE, MSG_Q_FIFO);
    OUT_CONTROL_QUEUE = msgQCreate(MAX_LOG_BUFF, MAX_LOG_SIZE, MSG_Q_FIFO);
    IN_DIAGNOSTICS_QUEUE = msgQCreate(MAX_LOG_BUFF, MAX_LOG_SIZE, MSG_Q_FIFO);
    OUT_DIAGNOSTICS_QUEUE = msgQCreate(MAX_LOG_BUFF, MAX_LOG_SIZE, MSG_Q_FIFO);

    WIFI_TID = taskSpawn("wifiTask", 50, 0, 20000,(FUNCPTR) wifiMain, 0,0,0,0,0,0,0,0,0,0);
    CONTROL_TID = taskSpawn("ctrlTask", 50, 0, 20000,(FUNCPTR) controlMain, 0,0,0,0,0,0,0,0,0,0);
    
    taskDeleteHookAdd((FUNCPTR)hookWifiDelete);
    taskDeleteHookAdd((FUNCPTR)hookControlDelete);
    // debug
    // memset(msg, 0, 100);
	// snprintf(msg, 100, "task_id dell'init task  : %i", INIT_TID);
	// logMessage(msg,taskName(0));

}

