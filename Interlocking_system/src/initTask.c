/*
 * initTask.c
 *
 *  Created on: Jan 3, 2023
 *      Author: alber
 */


#include "initTask.h"

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

network node_net;
time_t current_time = 0;
route *node_routes;


int parseConfigString(char* config_string,route **routes, network *net){
    //TODO: gestire gli errori durante il parsing

    char *packet[4];

    //tokenizzo e salvo tutte le tipologie di pacchetti

    for(int packet_idx = 0; packet_idx < 4; packet_idx++){
        if(packet_idx == 0){
            packet[packet_idx] = strtok(config_string, ";");
        }
        else {
            packet[packet_idx] = strtok(NULL, ";");
        }
        if(packet[packet_idx] == NULL)return(-1);
    }
    
    //analizzo il primo pacchetto
    
    sscanf(packet[0],"%li,%i,%i,%i", &current_time, &net->prev_node_count, &net->next_node_count, &net->route_count);

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

    //secondo pacchetto
    char *ip;
    
    for(int ip_idx = 0; ip_idx < net->prev_node_count; ip_idx++){
        if (ip_idx == 0){
            ip = strtok(packet[1], ",");
        }
        else{
            ip = strtok(NULL, ",");
        }
        if(ip == NULL)return(-1);

        strcpy(net->prev_ips[ip_idx],ip);
    }
    char* id;

    for(int id_idx = 0; id_idx < net->prev_node_count; id_idx++){
        id = strtok(NULL, ",");
        if(id == NULL)return(-1);
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
        if(ip == NULL)return(-1);

        strcpy(net->next_ips[ip_idx],ip);
    }

    for(int id_idx = 0; id_idx < net->next_node_count; id_idx++){
        id = strtok(NULL, ",");
        if(id == NULL)return(-1);
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
        if(route_data == NULL)return(-1);


        sscanf(route_data,"%i,%i,%i", &(*routes)[route_idx].route_id,
                                &(*routes)[route_idx].rasp_id_prev, &(*routes)[route_idx].rasp_id_next);
    }


    return(0);
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
	
	//apro la connessione con l'host per ricevere i dati di configurazione
	char HOST_IP[] = "192.168.1.202";
	// char HOST_IP[] = "172.23.78.0";
	connection host_s = {.fd=0, .sock=0, .connected_id=0};
	

	connectToServer(&host_s, HOST_IP, SERVER_PORT);
	char msg[50];
	snprintf(msg, 50, "RASP_ID : %i", RASP_ID);
	sendToConn(&host_s, msg);
	char config_string[1024] = {0};
	
	readFromConn(&host_s, config_string, 1024);
	if(parseConfigString(config_string, &node_routes, &node_net) == -1){
		printf("ERRORE nel parsing della config_string");
	}
    else{
        // fprintf(debug_file, "[RASP_ID : %i] Configurazione ricevuta\n", RASP_ID);
        // printf("[RASP_ID : %i] Configurazione ricevuta\n", RASP_ID);
        //printConfigInfo(node_routes, &node_net);
    }
    
    //Notifico l'host dell'avvenuta configurazione
    memset(msg, 0, 50);
    snprintf(msg, 50, "Configurazione eseguita su RASP_ID : %i", RASP_ID);
    sendToConn(&host_s, msg);
    
    //Prima di procedere attendo che l'host mi notifichi l'avvenuta configurazione di tutti i nodi
    memset(msg, 0, 50);
    readFromConn(&host_s, msg, 50);
    // fprintf(debug_file, "[RASP_ID : %i] %s\n", RASP_ID, msg);
    // printf("[RASP_ID : %i] : %s\n", RASP_ID, msg);
	

    //qui parte il protocollo per instaurare le connessioni dei nodi a catena
    //prima fase : client
    
    
    for(int node_idx=0; node_idx<node_net.prev_node_count; node_idx++){
        //tento di connettermi al nodo precedente ripetutamente finchè non apre la connessione
        int conn_status = 0;
        do{
            conn_status = addConnToServer(node_net.prev_ips[node_idx], SERVER_PORT, node_net.prev_ids[node_idx]);
        }while(conn_status == CONN_REFUSED);
    }
    

    //seconda fase : server

    //abbiamo assunto che nessuna route può terminare con uno scambio,
    //quindi tutti i nodi di terminazione avranno un solo nodo successivo con id : TAIL_ID

    if(node_net.next_node_count == 1 && node_net.next_ids[0] == TAIL_ID){
        // Caso nodo di terminazione
        memset(msg, 0, 50);
        snprintf(msg, 50, "RASP_ID : %i", RASP_ID);
        for(int node_idx=0; node_idx<node_net.prev_node_count; node_idx++){
            sendToConn(getConn(node_idx), msg);
        }
    }
    else{
        addConnToClient(node_net.next_node_count);
        // Attendo che tutti i nodi collegati notifichino l'avvenuta connessione
        for(int node_idx=0; node_idx<node_net.next_node_count; node_idx++){
            memset(msg, 0, 50);
            readFromConn(getConn(node_net.prev_node_count + node_idx), msg, 50);
            // fprintf(debug_file, "[RASP_ID : %i] %s ha stabilito tutte le sue connessioni\n", RASP_ID, msg);
        }
        // Ricevuta la notifica da tutt i nodi successivi , informo quelli precedenti
        memset(msg, 0, 50);
        snprintf(msg, 50, "RASP_ID : %i", RASP_ID);
        for(int node_idx=0; node_idx<node_net.prev_node_count; node_idx++){
            sendToConn(getConn(node_idx), msg);
        }
    }


    //TODO eliminare strutture e memoria allocata per il taskInit una volta concluso
    // fprintf(debug_file, "[RASP_ID : %i] Finito\n", RASP_ID);
    // printf("[RASP_ID : %i] Finito\n", RASP_ID);

}

