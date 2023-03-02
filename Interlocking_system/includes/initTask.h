/*
 * initTask.h
 *
 *  Created on: Jan 3, 2023
 *      Author: alber
 */

#ifndef INCLUDES_INITTASK_H_
#define INCLUDES_INITTASK_H_



#include "global_variables.h"
#include "wifiTask.h"
#include "logTask.h"

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
    int *prev_ids;
    int *next_ids;
}network;


extern void setCurrentTime(time_t current_time);
extern exit_number parseConfigString(char* config_string,route **routes, network *net);
extern void printConfigInfo(route *routes, network *net);
extern void initMain(void);


#endif /* INCLUDES_INITTASK_H_ */
