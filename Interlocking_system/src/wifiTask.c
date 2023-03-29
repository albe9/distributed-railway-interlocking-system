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

exit_number readFromConn(connection *conn, char* buffer, ssize_t buf_size){
	
	ssize_t byte_read;
	byte_read = recv(conn->sock, buffer, buf_size, 0);
	if(byte_read == 0){ return E_DISCONNECTION; }
	else {return E_SUCCESS;}
}

void hookWifiDelete(_Vx_TASK_ID tcb){

	if(tcb == WIFI_TID && total_conn > 0){
		resetConnections();
		logMessage("Task wifi eliminato",taskName(0));
	}

}

void resetConnections(){
	
		
	for(int i=0; i< total_conn; i++){
		shutdown(node_conn[i].sock, SHUT_RDWR);
		if (close(node_conn[i].sock) < 0){
			logMessage( errorDescription(E_DEFAUL_ERROR) ,taskName(0));
		}
		node_conn[i] = (const connection){0};
	}
	total_conn = 0;
	
}

int getSizeofLog(char *path_to_file){
	// Apriamo il file
	FILE *file;
	if((file = fopen(path_to_file, "r")) < 0){
		logMessage("Problema nell'apertura del file", taskName(0));
		return E_LOG_OPEN;
	}	
	if (file != NULL) {
    	/* Go to the end of the file. */
		if (fseek(file, 0L, SEEK_END) == 0) {
			/* Get the size of the file. */
			int bufsize = (int) ftell(file);
			if (bufsize == -1) {
				logMessage("File di dimensioni nulla", taskName(0));
				return E_LOG_EMPTY;
			}
			printf("bufsize %i \n", bufsize);
		fclose(file);
		return bufsize;
		}
	}
	else{
		logMessage("Puntatore nullo", taskName(0));
		return E_LOG_EMPTY;
	}
}

exit_number logToHost(void){
	// Si sospende il task che esegue il logInit
	if(taskSuspend(LOG_TID) < 0){
		return E_DEFAUL_ERROR;
	}
	// Si crea un socket verso l'host
	connection host_conn;
	connectToServer(&host_conn, HOST_IP, LOG_PORT);
	// Si legge quanto è lungo il file 
	int logSize = 0;
	if((logSize = getSizeofLog("/usr/log/log.txt")) > 0){
		// E si copia il contenuto in un buffer
		char *logMsg = malloc(sizeof(char) * (logSize + 1));
		printf("Allocazione eseguita \n");
		FILE *file = fopen("/usr/log/log.txt", "r");
		/* Go back to the start of the file. */
        if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }
        /* Read the entire file into memory. */
        size_t newLen = fread(logMsg, sizeof(char), logSize, file);
		printf("newLen %i \n", newLen);
        if ( ferror(file) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            logMsg[newLen++] = '\0'; /* Just to be safe. */
        }
		printf("%s", logMsg);
		fclose(file);
		// Si invia il messaggio
		sendToConn(&host_conn, logMsg);
		shutdown(host_conn.sock, SHUT_RDWR);
		if (close(host_conn.sock) < 0){
			return E_DEFAUL_ERROR;
		}
		taskResume(LOG_TID);
		return E_SUCCESS;
	}
	else{
		// Errore da gestire
		taskResume(LOG_TID);
		logMessage("Size del log nulla o negativa [errore]", taskName(0));
		return E_LOG_EMPTY;
	}

}

extern exit_number handle_inMsg(char* msg, int sender_id){

	/*	
		Sintassi messaggi:
			-relativi a route : "command;host_id;route_id"
			-relativi a ping : "TODO definire"
			-host close : "CLOSE;"
	
	*/
	//TODO definire lunghezza massima dei comandi

	

	char *command_type, *msg_data; 
	if((command_type = strtok(msg, ";")) == NULL)return(E_PARSING);
	else{
	    if((msg_data=strtok(NULL,""))==NULL)return(E_PARSING);
	}

	//gestisco solo alcuni tipi di messaggi, gli altri li inoltro al task di controllo

	if (strcmp(command_type, "PING_REQ") == 0 ){
		//TODO gestire ping rispondendo al rasp
	}
	else if (strcmp(msg, "CLOSE") == 0){
		//inoltro a tutti i nodi connessi di terminare
		for(int node_idx=0; node_idx<total_conn;node_idx++){
			if(node_conn[node_idx].connected_id != sender_id){
				char msg[]="CLOSE;";
				sendToConn(&node_conn[node_idx], msg);
			}
		}
		return E_CLOSE;
	}
	// TODO aggiungere altri messagi
	else{
		//gestiamo i messaggi relativi al TPCM
		tpcp_msg in_msg;
		in_msg.recevier_id=RASP_ID;
		in_msg.sender_id=sender_id;
		strcpy(in_msg.command, command_type);

		//debug
		// logMessage(in_msg.command, taskName(0));


		int msg_host = -1;
		if(sscanf(msg_data,"%i;%i", &msg_host, &in_msg.route_id) != 2)return(E_PARSING);
		//acquisisco il semaforo per accedere alla variabile globale (è condivisa con il task di controllo)
		semTake(GLOBAL_SEM, WAIT_FOREVER);
		//se non ci sono host correnti lo setto
		if(CURRENT_HOST == -1){
			CURRENT_HOST = msg_host;
		}

		if(CURRENT_HOST != msg_host){
			//TODO nodo già occupato, gestire invio di not ack ecc
		}
		else{
			//passo i dati al task di controllo
			in_msg.host_id = CURRENT_HOST;
			msgQSend(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
		}	
		semGive(GLOBAL_SEM);
	}


	return E_SUCCESS;
}

extern exit_number handle_outMsg(tpcp_msg* out_msg){

	//debug
	// char msg[100];
	// snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", out_msg->command, out_msg->sender_id, out_msg->recevier_id, out_msg->route_id);
	// logMessage(msg, taskName(0));

	for(int node_idx=0; node_idx<total_conn; node_idx++){
		if(node_conn[node_idx].connected_id == out_msg->recevier_id){
			char msg[100];
			snprintf(msg, 100,"%s;%i;%i", out_msg->command, out_msg->host_id, out_msg->route_id);
			sendToConn(&node_conn[node_idx], msg);
			return E_SUCCESS;
		}
	}

	//Se non trovo nessun nodo corrispondente significa che c'è stato un errore
	return E_NODE_NOTFOUND;

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
	

	bool flag_running=true;

	while(flag_running){
		// Da man select .....if using select() within a loop, the
		// sets must be reinitialized before each call.
		// Resetto e riaggiungo gli fds dei socket delle connessioni
		FD_ZERO(&readfds);
		for(int conn_idx=0; conn_idx<total_conn; conn_idx++){
			FD_SET(node_conn[conn_idx].sock, &readfds);
		}
		// resetto il timeout(viene modificato da select)
		struct timeval select_timeout={.tv_sec=0, .tv_usec=1000};
		// controlla senza bloccare se un socket riceve un msg
		n_ready_conn = select(nfds, &readfds, NULL, NULL, &select_timeout);
		if(n_ready_conn > 0){
			// controllo quali fd sono rimasti in readfds (quelli che hanno ricevuto un msg)
			for(int conn_idx=0; conn_idx<total_conn; conn_idx++){
				if(FD_ISSET(node_conn[conn_idx].sock, &readfds)){
					//debug
					// snprintf(msg, 100, "Ricevuto messaggio da Rasp id : %i", node_conn[conn_idx].connected_id);
					// logMessage(msg, taskName(0));
					// memset(msg, 0, 100);
					exit_number status;
					if((status = readFromConn(&node_conn[conn_idx], msg, 100)) == E_SUCCESS){
						if((status = handle_inMsg(msg, node_conn[conn_idx].connected_id)) == E_CLOSE){
							//se ho ricevuto un messaggio CLOSE dall'host termino il task
							flag_running=false;
						}
						else if(status == E_SUCCESS){
							continue;
						}
						else{
							logMessage(errorDescription(status), taskName(0));
						}
					}
					else if(status == E_DISCONNECTION){
						//Se un nodo interrompe la connessione loggo
						memset(msg, 0, 100);
						snprintf(msg, 100, "Disconnesso Rasp id : %i", node_conn[conn_idx].connected_id);
						logMessage(msg, taskName(0));
						// chiudo il socket relativo e rimuovo la sua connesione dall'array delle connessioni attive
						shutdown(node_conn[conn_idx].sock, SHUT_RDWR);
						if (close(node_conn[conn_idx].sock) < 0){
							logMessage("Impossibile chiudere il socket",taskName(0));
						}
						node_conn[conn_idx] = (const connection){0};
						total_conn -= 1;
						for(int j=conn_idx; j<total_conn;j++){
							node_conn[j] = node_conn[j+1];
						}
					}
				}
			}
		}
		else if(n_ready_conn == -1){
			logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
		}

		//gestisco i messaggi in uscita
		tpcp_msg out_msg;
		ssize_t byte_recevied = msgQReceive(OUT_CONTROL_QUEUE, (char*)&out_msg, sizeof(tpcp_msg), 1);
		if(byte_recevied > 0){
			//debug
			// char msg[100];
			// snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", out_msg.command, out_msg.sender_id, out_msg.recevier_id, out_msg.route_id);
			// logMessage(msg, taskName(0));
			exit_number status;
			if( (status = handle_outMsg(&out_msg)) != E_SUCCESS){
				logMessage(errorDescription(status), taskName(0));
			}
		}
        
	}


	resetConnections();
	logMessage("Terminato", taskName(0));



}








