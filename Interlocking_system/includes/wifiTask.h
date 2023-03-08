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
#include "logTask.h"
#include <sys/select.h>
// #include <epollLib.h>

#define CONN_REFUSED -2

typedef struct{
	int sock;
	int connected_id;
}connection;

//funzioni per la fase di config (init task)
extern exit_number addConnToServer(char* server_ip, int server_port, int server_id);
extern exit_number connectToServer(connection *conn_server, char* server_ip, int server_port);
extern exit_number addConnToClient(int num_client);

extern void setBlockingMode(bool blocking_mode);
extern connection* getConn(int conn_idx);
extern void sendToConn(connection *conn, char *msg);
extern ssize_t readFromConn(connection *conn, char* buffer, ssize_t buf_size);

extern void wifiMain(void);
extern exit_number handle_msg(char* msg);
extern exit_number resetConnections();

#endif /* INCLUDES_WIFITASK_H_ */
