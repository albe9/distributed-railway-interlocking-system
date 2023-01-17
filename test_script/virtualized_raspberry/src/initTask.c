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
    //quarto pacchetto
    char *route_data;
    for(int route_idx = 0; route_idx < net->route_count; route_idx++){
        if (route_idx == 0){
            route_data = strtok(packet[3], "-");
        }
        else{
            route_data = strtok(NULL, "-");
        }
        if(route_data == NULL)return(-1);


        sscanf(route_data,"%i,%i,%i", &(*routes)[route_idx].route_id,
                                &(*routes)[route_idx].rasp_id_prev, &(*routes)[route_idx].rasp_id_next);
    }


    return(0);
}

void printConfigInfo(route *routes, network *net){
	printf("Config del nodo:\n\t-Nodi precedenti :%i\n\t-Nodi successivi :%i\n\t-Routes :%i\n",
			net->prev_node_count,net->next_node_count,net->route_count);
	printf("Ip precendenti : \n");
	for(int ip_idx = 0;ip_idx < net->prev_node_count;ip_idx ++){
		printf("\t-%s\n",net->prev_ips[ip_idx]);
	}
	printf("Ip successivi : \n");
	for(int ip_idx = 0;ip_idx < net->next_node_count;ip_idx ++){
		printf("\t-%s\n",net->next_ips[ip_idx]);
	}
	printf("Routes : \n");
	for(int route_idx = 0;route_idx < net->route_count;route_idx++){
		printf("\t-Route id : %i\n\t-Node id precedente : %i\n\t-Node id successivo : %i\n\n",
				routes[route_idx].route_id, routes[route_idx].rasp_id_prev, routes[route_idx].rasp_id_next);
	}
	
}

void initMain(void){
	
	//apro la connessione con l'host per ricevere i dati di configurazione
	int HOST_PORT = 6543;
	// char HOST_IP[] = "192.168.1.35";
	char HOST_IP[] = "172.23.78.0";
	conn conn_host = {.fd = 0, .sock = 0};
	

	connectToServer(&conn_host, HOST_IP, HOST_PORT);
	char msg[50];
	snprintf(msg, 50, "Connessione da raspberry con RASP_ID : %i", RASP_ID);
	sendToServer(&conn_host, msg);
	char config_string[1024] = {0};
	
	readFromServer(&conn_host, config_string, 1024);
	if(parseConfigString(config_string, &node_routes, &node_net) == -1){
		printf("ERRORE nel parsing della config_string");
	}
	
	printConfigInfo(node_routes, &node_net);
}

