/*
 * wifiTask.c
 *
 *  Created on: Jan 4, 2023
 *      Author: alber
 */


#include "wifiTask.h"

//variabili statiche (scope locale) per il taskWifi
static connection node_conn[MAX_CONN];
static int total_conn = 0;
static bool flag_blocking = true;

exit_number connectToServer(connection *conn_server, char* server_ip, int server_port){
	//TODO gestire errori ed assegnare codici da ritornare per tutti i casi
	
	struct sockaddr_in serv_addr;
	
	if ((conn_server->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return E_DEFAUL_ERROR;
	}
	
	//linux
	// struct sockaddr_in local_addr;
	// local_addr.sin_family = AF_INET;
	// local_addr.sin_addr.s_addr = inet_addr(RASP_IP);
	// local_addr.sin_port = 0;
		
	//Bind
	// if( bind(conn_server->sock ,(struct sockaddr *)&local_addr , sizeof(local_addr)) < 0)
	// {
	// 	close(conn_server->sock);
	// 	return E_DEFAUL_ERROR;
	// }

	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);
 
 
	if (connect(conn_server->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		if(errno == ECONNREFUSED){
			close(conn_server->sock);
			return(E_CONN_REFUSED);
		}
		return E_DEFAUL_ERROR;
	}
	
	return E_SUCCESS;
}

exit_number addConnToServer(char* server_ip, int server_port, int server_id){
	if(total_conn < MAX_CONN - 1){
		//tento di connettermi al server, in caso di errore lo restituisco
		int conn_status = connectToServer(&node_conn[total_conn], server_ip, server_port);
		if(conn_status){
			return conn_status;
		}
		char msg[100];
		// fprintf(debug_file, "[RASP_ID : %i] connesso al server %s\n", RASP_ID, server_ip);
		// printf("[RASP_ID : %i] connesso al server %s\n", RASP_ID, server_ip);
		snprintf(msg, 100, "Connesso al server %s", server_ip);
		logMessage(msg, taskName(0));

		memset(msg, 0, 100);
		snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
		sendToConn(&node_conn[total_conn], msg);

		memset(msg, 0, 100);
		int returned_id;
		readFromConn(&node_conn[total_conn], msg, 100);
		sscanf(msg, "RASP_ID : %i", &returned_id);
		if(returned_id != server_id){
			//printf("Errore id server non corrispondente\nid aspettato : %i id ricevuto : %i\n", server_id, returned_id);
			memset(msg, 0, 100);
			snprintf(msg, 100, "Errore id server non corrispondente\nid aspettato : %i id ricevuto : %i", server_id, returned_id);
			logMessage(msg, taskName(0));
			//TODO gestire errore
			return E_INVALID_ID;
		}
		else{
			// fprintf(debug_file, "[RASP_ID : %i] Server id : %i aggiunto correttamente\n", RASP_ID, returned_id);
			memset(msg, 0, 100);
			snprintf(msg, 100, "Server id : %i aggiunto correttamente", server_id);
			logMessage(msg, taskName(0));
			node_conn[total_conn].connected_id = server_id;
			total_conn++;
			return E_SUCCESS;
		}
	}
	else{
		printf("Errore raggiunto numero massimo di socket per questo nodo\n");
		//TODO gestire errore
		return E_MAX_CONNECTION_NUMBER;
	}
	
}

extern exit_number addConnToClient(int num_client){

	int server_sock, addrlen;
	struct sockaddr_in server, client;
	
	//Creo il server_socket responsabile di accettare le connessioni in ingresso da altri nodi
	
	if ((server_sock = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		return E_DEFAUL_ERROR;
	}
	
	//Setto le opzioni del socket affinchè possa riutilizzare la stessa porta(e indirizzo)
	int enable = 1;
	if (setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int)) < 0)
	{
		perror("Setsockopt");
	}
	if (setsockopt(server_sock,SOL_SOCKET,SO_REUSEPORT,&enable,sizeof(int)) < 0)
	{
		perror("Setsockopt");
	}
	
	server.sin_addr.s_addr = inet_addr(RASP_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons( 6543 );
		
	//Bind
	if( bind(server_sock,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		return E_DEFAUL_ERROR;
	}
	
	//TODO definire il numero massimo di connessioni in coda
	listen(server_sock , 10);
	// fprintf(debug_file, "[RASP_ID : %i] sto aspettando connessioni\n", RASP_ID);
	logMessage("Sto aspettando connessioni", taskName(0));


	addrlen = sizeof(struct sockaddr_in);
	
	
	
	for(int client_idx=0; client_idx<num_client;client_idx++){
		if(total_conn < MAX_CONN - 1){

			//tento di accettare un client
			if ((node_conn[total_conn].sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t*)&addrlen)) < 0)
			{
				return E_DEFAUL_ERROR;
			}

			
			char host_addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client.sin_addr), host_addr, INET_ADDRSTRLEN);
			// fprintf(debug_file, "[RASP_ID : %i] ricevuta connessione da : %s\n", RASP_ID, host_addr);
			char msg[100];
			snprintf(msg, 100, "Ricevuta connessione da : %s", host_addr);
			logMessage(msg, taskName(0));


			
			int client_id;
			memset(msg, 0, 100);
			readFromConn(&node_conn[total_conn], msg, 100);
			
			sscanf(msg,"RASP_ID : %i", &client_id);
			
			memset(msg, 0, 100);
			snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
			sendToConn(&node_conn[total_conn], msg);
			// fprintf(debug_file, "[RASP_ID : %i] Client id : %i aggiunto correttamente\n", RASP_ID, client_id);
			memset(msg, 0, 100);
			snprintf(msg, 100, "Client id : %i aggiunto correttamente", client_id);
			logMessage(msg, taskName(0));
			node_conn[total_conn].connected_id = client_id;
			total_conn++;
			
		}
		else{
			return E_MAX_CONNECTION_NUMBER;
			//TODO gestire errore
		}
	}
	

	shutdown(server_sock, SHUT_RDWR);
	if (close(server_sock) < 0){
		return E_DEFAUL_ERROR;
	}

	return E_SUCCESS;
}

void setBlockingMode(bool blocking_mode){
	flag_blocking = blocking_mode;
}

connection* getConn(int conn_idx){
	return(&node_conn[conn_idx]);
}

void sendToConn(connection *conn, char *msg){
	
	send(conn->sock, msg, strlen(msg), 0);
}

ssize_t readFromConn(connection *conn, char* buffer, ssize_t buf_size){
	
	ssize_t valread;
	do{
		valread = recv(conn->sock, buffer, buf_size, 0);
	}while(flag_blocking && valread == 0);
	return(valread);
}

exit_number resetConnections(){
	
	for(int i=0; i< total_conn; i++){
		shutdown(node_conn[i].sock, SHUT_RDWR);
		if (close(node_conn[i].sock) < 0){
			return E_DEFAUL_ERROR;
		}
	}
	

	return E_SUCCESS;
}


void wifiMain(void){
	

	//tento monitoraggio dei socket con select

	char msg[100];

	// Determino il fd con numero maggiore
	int nfds=0;
	for(int fd_idx=0; fd_idx < total_conn; fd_idx++){
		if(nfds < node_conn[fd_idx].sock){
			nfds = node_conn[fd_idx].sock;
		}
	}
	nfds += 1;
	

	// Dichiaro il set per gli fds delle connessioni
	fd_set readfds;

	int n_ready_conn=0;
	

	while(true){
		// Da man select .....if using select() within a loop, the
		// sets must be reinitialized before each call.
		// Resetto e riaggiungo gli fds dei socket delle connessioni
		FD_ZERO(&readfds);
		for(int conn_idx=0; conn_idx<total_conn; conn_idx++){
			FD_SET(node_conn[conn_idx].sock, &readfds);
		}
		// blocca finchè almeno un socket non riceve un msg
		n_ready_conn = select(nfds, &readfds, NULL, NULL, NULL);
		// controllo quali fd sono rimasti in readfds (quelli che hanno ricevuto un msg)
		for(int conn_idx=0; conn_idx<total_conn; conn_idx++){
			if(FD_ISSET(node_conn[conn_idx].sock, &readfds)){
				snprintf(msg, 100, "Ricevuto messaggio da Rasp id : %i", node_conn[conn_idx].connected_id);
				logMessage(msg, taskName(0));
				memset(msg, 0, 100);
				readFromConn(&node_conn[conn_idx], msg, 100);
				logMessage(msg, taskName(0));
				memset(msg, 0, 100);
			}
		}



	}


}







