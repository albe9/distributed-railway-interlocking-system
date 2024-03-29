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
#include "gpio.h"
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
extern connection* getConnByIndex(int conn_idx);
extern connection* getConnByID(int rasp_id);
extern void sendToConn(connection *conn, char *msg);
extern exit_number readFromConn(connection *conn, char* buffer, ssize_t buf_size);
extern void processLogToSend(void);
extern void sendLogToHost(void);



extern void wifiMain(void);
extern exit_number handleInSingleMsg(char* msg, int sender_id);
extern exit_number handleInMsgs(char* msg, int sender_id);
extern exit_number handleOutControlMsg(tpcp_msg* out_control_msg, bool flag_log_not_ok);
extern exit_number checkDiag();
extern void wifiDestructor(int sig);
extern void resetConnections();

#endif /* INCLUDES_WIFITASK_H_ */
