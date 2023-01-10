#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
    test data:
    first msg: 
        -"1673375234,2"
    second msg:
        -"1,0,2,127.0.0.1,127.0.0.1;2,0,2,127.0.0.1,127.0.0.1;"

*/

typedef struct{
    int route_id;
    int rasp_id_prev;
    int rasp_id_next;
    char prev_ip[20];
    char next_ip[20];
}route;

int main(){


    char config_data[] = "1673375234,2;1,0,2,127.0.0.1,127.0.0.1;2,0,2,127.0.0.1,127.0.0.1;";

    char *token;

    token = strtok(config_data, ";");
    

    time_t current_time = 0;
    int route_count = 0;

    sscanf(token,"%i,%i", &current_time, &route_count);

    printf("Dati : %i %i",current_time,route_count);

    
    
    route *routes = (route*)malloc(route_count * sizeof(route));

    
    for(int i=0;i<route_count;i++){
        token = strtok(NULL, ";");
        sscanf(token,"%i,%i,%i,%[0-9.],%[0-9.]",&(routes + i)->route_id,&(routes + i)->rasp_id_prev,
            &(routes + i)->rasp_id_next,(routes + i)->prev_ip,(routes + i)->next_ip);
        
    }

    free(routes);
    
   
    return(0);
    
}