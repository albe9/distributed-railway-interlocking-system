/*
 * wifiTask.h
 *
 *  Created on: Jan 4, 2023
 *      Author: alber
 */

#ifndef INCLUDES_WIFITASK_H_
#define INCLUDES_WIFITASK_H_

#include <arpa/inet.h>
#include <sockLib.h>

#include "global_variables.h"


extern void addServer(char* server_ip, int server_port);
extern void sendToServer(char *msg);
extern void readFromServer(char* buffer, ssize_t buf_size);

extern void setClient(void);
extern void wifiMain(void);
extern void removeServer(void);

#endif /* INCLUDES_WIFITASK_H_ */
