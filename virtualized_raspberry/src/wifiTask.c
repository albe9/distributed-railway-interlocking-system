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

int connectToServer(connection *conn_server, char* server_ip, int server_port){
	//TODO gestire errori ed assegnare codici da ritornare per tutti i casi
	
	struct sockaddr_in serv_addr;
	
	if ((conn_server->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("\nErrore nella creazione del socket");
	}
	
	//linux
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr(RASP_IP);
	local_addr.sin_port = 0;
		
	//Bind
	if( bind(conn_server->sock ,(struct sockaddr *)&local_addr , sizeof(local_addr)) < 0)
	{
		perror("Errore durante il Bind");
		close(conn_server->sock);
	}

	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);
 
 
	if ((conn_server->fd = connect(conn_server->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		if(errno == ECONNREFUSED){
			close(conn_server->sock);
			return(CONN_REFUSED);
		}
		perror("\nErrore connessione fallita");
	}
	
	return(0);
}

int addConnToServer(char* server_ip, int server_port, int server_id){
	
	if(total_conn < MAX_CONN - 1){
		//tento di connettermi al server, in caso di errore lo restituisco
		int conn_status = connectToServer(&node_conn[total_conn], server_ip, server_port);
		if(conn_status){
			return(conn_status);
		}
		printf("[RASP_ID : %i] connesso al server %s\n", RASP_ID, server_ip);

		char msg[20];
		snprintf(msg, 20, "RASP_ID : %i", RASP_ID);
		sendToConn(&node_conn[total_conn], msg);

		memset(msg, 0, 20);
		char *returned_id;
		readFromConn(&node_conn[total_conn], msg, 20);
		returned_id = strtok(msg, ":");
		returned_id = strtok(NULL, ":");
		if(returned_id == NULL || atoi(returned_id) != server_id){
			printf("Errore id server non corrispondente\nid aspettato : %i id ricevuto : %i\n", server_id, atoi(returned_id));
			//TODO gestire errore
		}
		else{
			printf("[RASP_ID : %i] Server id : %i aggiunto correttamente\n", RASP_ID, server_id);
			node_conn[total_conn].connected_id = server_id;
			total_conn++;
			return(0);
		}
	}
	else{
		printf("Errore raggiunto numero massimo di socket per questo nodo\n");
		//TODO gestire errore
	}
}

extern int addConnToClient(){
	if(total_conn < MAX_CONN - 1){
		//tento di connettermi al client, in caso di errore lo restituisco
		int conn_status = connectToClient(&node_conn[total_conn]);
		if(conn_status){
			return(conn_status);
		}
		char msg[20];
		char *returned_id;
		readFromConn(&node_conn[total_conn], msg, 20);
		returned_id = strtok(msg, ":");
		returned_id = strtok(NULL, ":");
		if(returned_id == NULL){
			printf("Errore id client non ricevuto correttamente\n");
			//TODO gestire errore
		}
		else{
			int client_id = atoi(returned_id);
			memset(msg, 0, 20);
			snprintf(msg, 20, "RASP_ID : %i", RASP_ID);
			sendToConn(&node_conn[total_conn], msg);
			printf("[RASP_ID : %i] Client id : %i aggiunto correttamente\n", RASP_ID, client_id);
			node_conn[total_conn].connected_id = client_id;
			total_conn++;
			return(0);
		}
	}
	else{
		printf("Errore raggiunto numero massimo di socket per questo nodo\n");
		//TODO gestire errore
	}
}
extern int connectToClient(connection *conn_client){

	int server_sock, addrlen;
	struct sockaddr_in server, client;
	
	//Creo il server_socket responsabile di accettare le connessioni in ingresso da altri nodi
	
	if ((server_sock = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		printf("\nImpossibile creare il socket\n");
		perror("Errore ");
	}
	
	//Setto le opzioni del socket affinchè possa riutilizzare la stessa porta(e indirizzo)
	int reuse_addr = 1;
	if (setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,&reuse_addr,sizeof(int)) < 0)
	{
		perror("Setsockopt");
	}
	
	server.sin_addr.s_addr = inet_addr(RASP_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons( 6543 );
		
	//Bind
	if( bind(server_sock,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("Errore durante il Bind");
	}
	
	//TODO definire il numero massimo di connessioni in coda
	printf("[RASP_ID : %i] sto aspettando connessioni\n", RASP_ID);
	listen(server_sock , 1);

	addrlen = sizeof(struct sockaddr_in);
	if ((conn_client->sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t*)&addrlen)) < 0)
	{
		perror("accept failed");
	}

	
	char host_addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client.sin_addr), host_addr, INET_ADDRSTRLEN);
	printf("[RASP_ID : %i] ricevuta connessione da : %s\n", RASP_ID, host_addr);
	
	shutdown(server_sock, SHUT_RDWR);
	if (close(server_sock) < 0){
		perror("Errore chiusura server socket");
	}

	return(0);
}

void sendToConn(connection *conn, char *msg){
	
	send(conn->sock, msg, strlen(msg), 0);
}

void readFromConn(connection *conn, char* buffer, ssize_t buf_size){
	
	ssize_t valread;
	valread = recv(conn->sock, buffer, buf_size, 0);
}

void removeServer(connection *conn_server){
	// closing the connected socket
	close(conn_server->fd);
}


void wifiMain(void){
	
}







