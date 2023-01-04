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

typedef struct{
	int sock;
	int fd;
}conn;

extern void addServer(conn *conn_server, char* server_ip, int server_port);
extern void connectToServer(conn *conn_server, char* server_ip, int server_port);
extern void sendToServer(conn *conn_server, char *msg);
extern void readFromServer(conn *conn_server, char* buffer, ssize_t buf_size);

extern void setClient(void);
extern void wifiMain(void);
extern void removeServer(conn *conn_server);

#endif /* INCLUDES_WIFITASK_H_ */
