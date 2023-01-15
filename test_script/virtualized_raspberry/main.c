#include "global_variables.h"
#include "wifiTask.h"
#include "initTask.h"

#define IP_LEN 20

typedef struct{
    int route_id;
    int rasp_id_prev;
    int rasp_id_next;
}route;

typedef struct{
    int prev_node_count;
    int next_node_count;
    int route_count;
    char **prev_ips;
    char **next_ips;
}network;

network node_net;
time_t current_time = 0;
route *node_routes;


int parseConfigString(char* config_string,route *routes, network *net){
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

    routes = (route*)malloc(net->route_count * sizeof(route));

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

        sscanf(route_data,"%i,%i,%i", &routes[route_idx].route_id,
                                &routes[route_idx].rasp_id_prev, &routes[route_idx].rasp_id_next);
    }

    printf("finito\n");

    return(0);
}

int main(){

    //parseConfigString(config_data, node_routes, &node_net);
    initMain();
   
    return(0);
    
}

