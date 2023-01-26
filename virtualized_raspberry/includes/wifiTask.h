/*
 * wifiTask.h
 *
 *  Created on: Jan 4, 2023
 *      Author: alber
 */

#ifndef INCLUDES_WIFITASK_H_
#define INCLUDES_WIFITASK_H_


#include <arpa/inet.h>

#include "global_variables.h"

#define CONN_REFUSED -2

typedef struct{
	int sock;
	int fd;
	int connected_id;
}connection;


extern int addConnToServer(char* server_ip, int server_port, int server_id);
extern int connectToServer(connection *conn_server, char* server_ip, int server_port);
extern int addConnToClient(int num_client);

extern void sendToConn(connection *conn, char *msg);
extern void readFromConn(connection *conn, char* buffer, ssize_t buf_size);

extern void wifiMain(void);
extern void removeServer(connection *conn_server);

#endif /* INCLUDES_WIFITASK_H_ */
