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
bool log_debug_mode = false;

exit_number connectToServer(connection *conn_server, char* server_ip, int server_port){
	
	struct sockaddr_in serv_addr;
	
	if ((conn_server->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return E_DEFAUL_ERROR;
	}

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
		char log_msg[100];

		// debug
		// memset(log_msg, 0, 100);
		// snprintf(log_msg, 100, "Connesso al server %s", server_ip);
		// logMessage(log_msg, taskName(0));

		memset(msg, 0, 100);
		snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
		sendToConn(&node_conn[total_conn], msg);

		memset(msg, 0, 100);
		int returned_id;
		readFromConn(&node_conn[total_conn], msg, 100);
		sscanf(msg, "RASP_ID : %i", &returned_id);
		if(returned_id != server_id){
			memset(log_msg, 0, 100);
			snprintf(log_msg, 100, "Errore id server non corrispondente\nid aspettato : %i id ricevuto : %i", server_id, returned_id);
			logMessage(log_msg, taskName(0));
			return E_INVALID_ID;
		}
		else{
			memset(log_msg, 0, 100);
			snprintf(log_msg, 100, "Server id : %i aggiunto correttamente", server_id);
			logMessage(log_msg, taskName(0));
			node_conn[total_conn].connected_id = server_id;
			total_conn++;
			return E_SUCCESS;
		}
	}
	else{
		logMessage("Errore raggiunto numero massimo di socket per questo nodo", taskName(0));
		return E_MAX_CONNECTION_NUMBER;
	}
	
}

exit_number addConnToClient(int num_client){

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
	
	// Effettuo il listen sul socket server con un buffer di 10 connessioni
	listen(server_sock , 10);
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
			char msg[100];
			char log_msg[100];

			// debug
			// memset(log_msg, 0, 100);
			// snprintf(log_msg, 100, "Ricevuta connessione da : %s", host_addr);
			// logMessage(log_msg, taskName(0));


			
			int client_id;
			memset(msg, 0, 100);
			readFromConn(&node_conn[total_conn], msg, 100);
			
			sscanf(msg,"RASP_ID : %i", &client_id);
			
			memset(msg, 0, 100);
			snprintf(msg, 100, "RASP_ID : %i", RASP_ID);
			sendToConn(&node_conn[total_conn], msg);

			memset(log_msg, 0, 100);
			snprintf(log_msg, 100, "Client id : %i aggiunto correttamente", client_id);
			logMessage(log_msg, taskName(0));
			node_conn[total_conn].connected_id = client_id;
			total_conn++;
			
		}
		else{
			return E_MAX_CONNECTION_NUMBER;
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

/* Questa funzione prende un indice e ritorna la conn
del relativo indice in node_conn.

NOTA: da non confondere con getConnByID */
connection* getConnByIndex(int conn_idx){
	return(&node_conn[conn_idx]);
}

/* Questa funzione prende l'ID di un raspberry e cerca
tra le connessioni attive (ovvero node_conn) se una di queste
è stata creata da un raspberry con stesso ID e in caso positivo
restituisce tale conn.

NOTA: da non confondere con getConnByIndex */
connection* getConnByID(int rasp_id){
	connection* conn = NULL;
	for(int idx=0; idx<total_conn; idx++){
			if(node_conn[idx].connected_id == rasp_id){
				conn = &node_conn[idx];
				return conn;
			}
		}
	logMessage("Nessun rasp ID presente uguale a quello cercato, ritorno un connection* null", taskName(0));
	return conn;
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

void wifiDestructor(int sig){

	resetConnections();
	logMessage("Task wifi eliminato",taskName(0));
	taskDelete(0);
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

exit_number getSizeofLog(char *path_to_file, int* size){
	// Apriamo il file
	FILE *file;
	if((file = fopen(path_to_file, "r")) < 0){
		return E_LOG_OPEN;
	}	
	if (file != NULL) {
    	/* Go to the end of the file. */
		if (fseek(file, 0L, SEEK_END) == 0) {
			/* Get the size of the file. */
			int bufsize = (int) ftell(file);
			if (bufsize == -1) {
				return E_LOG_EMPTY;
			}
			if (log_debug_mode)
				printf("bufsize %i \n", bufsize);
			fclose(file);
			*size = bufsize;
			return E_SUCCESS;
		}
		else{
			return E_DEFAUL_ERROR;
		}
	}
	else{
		return E_LOG_EMPTY;
	}
}

exit_number processLogToSend(void){
	// Si sospende il task che esegue il logInit
	if(taskSuspend(LOG_TID) < 0){
		return E_DEFAUL_ERROR;
	}
	// Si crea un socket verso l'host
	connection host_conn;
	connectToServer(&host_conn, HOST_IP, LOG_PORT);
	// Si legge quanto è lungo il file 
	int logSize = 0;
	exit_number status;
	if((status = getSizeofLog("/usr/log/log.txt", &logSize)) != E_SUCCESS){
		return status;
	}
	// E si copia il contenuto in un buffer
	char *logMsg = malloc(sizeof(char) * (logSize + 1));
	if (log_debug_mode)
		printf("Allocazione eseguita \n");
	FILE *file = fopen("/usr/log/log.txt", "r");
	/* Go back to the start of the file. */
	if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }
	/* Read the entire file into memory. */
	size_t newLen = fread(logMsg, sizeof(char), logSize, file);
	if (log_debug_mode)
		printf("newLen %i \n", newLen);
	if ( ferror(file) != 0 ) {
		fputs("Error reading file", stderr);
	} else {
		logMsg[newLen++] = '\0'; /* Just to be safe. */
	}
	if (log_debug_mode)
		printf("%s", logMsg);
	fclose(file);
	// Si invia il messaggio
	sendToConn(&host_conn, logMsg);
	// Liberiamo la memoria allocata
	free(logMsg);
	// TODO: capire meglio shutdown e close
	shutdown(host_conn.sock, SHUT_RDWR);
	if (close(host_conn.sock) < 0){
		return E_DEFAUL_ERROR;
	}
	// Riavviamo il task di log e loggiamo il completamento dell'invio
	printf("Socket di log chiuso, riavvio task di log \n");
	if((taskResume(LOG_TID) < 0)){
		return E_RESUME_LOG;
	}
	return E_SUCCESS;
}

void sendLogToHost(void){
	exit_number status;
	if((status = processLogToSend()) != E_SUCCESS){
		logMessage(errorDescription(status), taskName(0));
	}
	else{
		logMessage("Inviato con successo il log all'host", taskName(LOG_TID));
	}
}

exit_number handleInMsgs(char* msg, int sender_id){

	/*
		Sintassi messaggi nei socket
		"singleMsg.singleMsg." ecc
	*/

	char **single_msg_array;
	int single_msg_count = 0;
	//Conto il numero di messaggi nel socket
	for(int ch_idx=0; ch_idx<strlen(msg); ch_idx++){
	    if(msg[ch_idx] == '.')single_msg_count++;
	}

	if(single_msg_count == 0)return E_PARSING;
	
	single_msg_array = (char**)malloc(single_msg_count * sizeof(char*));
	
	//Effettuo una tokenizazione per dividere i messaggi in singleMsg
	for(int msg_idx=0; msg_idx<single_msg_count; msg_idx++){
	    if(msg_idx == 0){
	        single_msg_array[msg_idx] = strtok(msg, ".");
	    }
	    else{
	        single_msg_array[msg_idx] = strtok(NULL, ".");
	    }
		// Controllo che la tokeninzzazione sia avvenuta correttamente
		if(single_msg_array[msg_idx] == NULL)return(E_PARSING);
	}

	exit_number status;
    for(int msg_idx=0; msg_idx<single_msg_count; msg_idx++){
        if((status = handleInSingleMsg(single_msg_array[msg_idx], sender_id)) == E_CLOSE)return E_CLOSE;
		else if(status != E_SUCCESS){
			logMessage(errorDescription(status), taskName(0));
		}
    }
	logMessage("[t49] Preselection WiFi no msg", taskName(0));
	logMessage("[t26] Non ci sono messaggi da gestire", taskName(0));
	return E_SUCCESS;
}

exit_number handleInSingleMsg(char* msg, int sender_id){

	/*	
		Sintassi messaggi:
			-relativi a route : "command;host_id;route_id"
			-relativi a ping : "PING_REQ;"
			-host close : "CLOSE;"
	
	*/

	

	char *command_type, *msg_data;
	// Nel caso il messaggio sia vuoto ritorna un errore di parsing
	if((command_type = strtok(msg, ";")) == NULL){
		return(E_PARSING_EMPTY);
	}
	/* 	Nel caso il campo di comando NON sia vuoto, ma è vuoto il successivo campo dati e
		questo campo è necessario (caso di REQ, WAIT_ACK, ...) ritorna un errore di parsing
		NOTA: nel caso di PING_REQ, PING_ACK e CLOSE non serve il campo dati*/
	else{
	    if((msg_data=strtok(NULL,"")) == NULL){
	        printf("%i,   %i\n", strcmp(command_type, "PING_REQ"), strcmp(command_type, "PING_ACK"));
			if (!((strcmp(command_type, "PING_REQ") == 0) || (strcmp(command_type, "PING_ACK") == 0))){
				return(E_PARSING_DATA_EMPTY);
			}
		}

    }

	//Parsing eseguito, continuo gestendo solo alcuni tipi di messaggi, gli altri li inoltro al task di controllo

	
	if (strcmp(command_type, "PING_REQ") == 0 ){
		// Trovo la connesione che mi ha inviato il ping
		connection* conn_ping = getConnByID(sender_id);
		char tmp_ping_msg[100];
		sprintf(tmp_ping_msg, "-----Ricevuto comando PING_REQ da nodo %i", sender_id);
		logMessage(tmp_ping_msg, taskName(0));
		memset(tmp_ping_msg, 0, 100);		
		//Rispondo al ping segnalando di essere un nodo attivo
		sendToConn(conn_ping, "PING_ACK;.");
		logMessage("[t48] Preselection WiFi msg diagnostica", taskName(0));
		sprintf(tmp_ping_msg, "-----[t11] Inviato comando PING_ACK al nodo %i", sender_id);
		logMessage(tmp_ping_msg, taskName(0));
		memset(tmp_ping_msg, 0, 100);
	}
	else if (strcmp(command_type, "PING_ACK") == 0){
		char tmp_ping_msg_2[100];
		logMessage("[t48] Preselection WiFi msg diagnostica", taskName(0));
		sprintf(tmp_ping_msg_2, "-----[t11] Ricevuto comando PING_ACK da nodo %i", sender_id);
		logMessage(tmp_ping_msg_2, taskName(0));
		memset(tmp_ping_msg_2, 0, 100);
		// Se ricevo un PING_ACK e la procedura di ping è in corso aumento il contatore
		if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0)return E_DEFAUL_ERROR;
		switch (ping_status)
		{
			case ACTIVE:
				if(ping_success){
					// Abbiamo ricevuto più PING_ACK del numero di connessioni attive
					return E_PING_ACK;
				}
				else{
					// Aumento il contatore
					ping_answers += 1;
					// Se raggiungiamo il numero di PING_ACK giusto indichiamo il successo della procedura di ping al task di diagnostica
					if (ping_answers == total_conn){
						ping_success = TRUE;
					}
				}
				break;
			case ENDING:
				// Azzeriamo il contatore delle risposte al ping e indichiamo che la procedura di ping è disattivata
				ping_answers = 0;
				ping_status = NOT_ACTIVE;
				break;
			default:
				//Se siamo in STARTING o NOT_ACTIVE non faccio nulla
				break;
		}
		if(semGive(WIFI_DIAG_SEM) < 0)return E_DEFAUL_ERROR;
	}
	else if (strcmp(command_type, "CLOSE") == 0){
		//inoltro a tutti i nodi connessi di terminare
		for(int node_idx=0; node_idx<total_conn;node_idx++){
			if(node_conn[node_idx].connected_id != sender_id){
				char msg[]="CLOSE;.";
				sendToConn(&node_conn[node_idx], msg);
			}
		}
		return E_CLOSE;
	}
	else{
		//gestiamo i messaggi relativi al TPCP
		tpcp_msg in_msg;
		
		//debug
		// logMessage(in_msg.command, taskName(0));


		int msg_host = -1;
		if(sscanf(msg_data,"%i;%i", &msg_host, &in_msg.route_id) != 2)return(E_PARSING);
		//acquisisco il semaforo per accedere alla variabile globale (è condivisa con il task di controllo)
		semTake(WIFI_CONTROL_SEM, WAIT_FOREVER);
		//se non ci sono host correnti lo setto
		if(CURRENT_HOST == -1){
			CURRENT_HOST = msg_host;
		}

		if(CURRENT_HOST != msg_host){
			in_msg.recevier_id = sender_id;
			in_msg.sender_id = RASP_ID;
			strcpy(in_msg.command, "NOT_OK");
			in_msg.host_id = msg_host;
			handleOutControlMsg(&in_msg);
		}
		else{
			//passo i dati al task di controllo
			in_msg.recevier_id=RASP_ID;
			in_msg.sender_id=sender_id;
			strcpy(in_msg.command, command_type);
			in_msg.host_id = CURRENT_HOST;
			logMessage("[t29] Preselection WiFi msg controllo", taskName(0));
			logMessage("[t8] filtraggio messaggio concluso", taskName(0));
			logMessage("[t6] acquisisco il semaforo per la coda", taskName(0));
			msgQSend(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
			logMessage("[t40] sposto messaggio dalla coda locale a quella globale", taskName(0));
			logMessage("[t23] rilascio semaforo", taskName(0));
		}	
		semGive(WIFI_CONTROL_SEM);
	}


	return E_SUCCESS;
}

exit_number handleOutControlMsg(tpcp_msg* out_control_msg){

	//debug
	char log_msg[100];
	snprintf(log_msg, 100, "Prima di inoltrare il msg, command :%s sender :%i recivier:%i route:%i", out_control_msg->command, out_control_msg->sender_id, out_control_msg->recevier_id, out_control_msg->route_id);
	logMessage(log_msg, taskName(0));

	for(int node_idx=0; node_idx<total_conn; node_idx++){
		if(node_conn[node_idx].connected_id == out_control_msg->recevier_id){
			char msg[100];
			// Riformatto il pacchetto seguendo la sintassi definita in handleInSingleMsg
			snprintf(msg, 100,"%s;%i;%i.", out_control_msg->command, out_control_msg->host_id, out_control_msg->route_id);
			sendToConn(&node_conn[node_idx], msg);
			logMessage("[t10] Messaggio inviato", taskName(0));
			return E_SUCCESS;
		}
	}

	//Se non trovo nessun nodo corrispondente significa che c'è stato un errore
	return E_NODE_NOTFOUND;

}

exit_number checkDiag(){
	logMessage("[t54] Preselection inizio ciclo Task WiFi", taskName(0));
	logMessage("-----[t18] acquisisco semaforo", taskName(0));
	if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0)return E_DEFAUL_ERROR;
	logMessage("-----[t32] controllo area di memoria", taskName(0));
	switch (ping_status)
	{
		case STARTING:
			// Inviamo un messaggio PING_REQ a tutti i nodi vicini
			for(int node_idx=0; node_idx<total_conn; node_idx++){
				sendToConn(&node_conn[node_idx], "PING_REQ;.");
			}
			logMessage("-----[t31] Inviato messaggi PING_REQ a tutti i vicini", taskName(0));
			// Cambiamo lo stato del ping
			ping_status = ACTIVE;
			break;
		case ENDING:
			// Azzeriamo il contatore delle risposte al ping e indichiamo che la procedura di ping è disattivata
			ping_answers = 0;
			ping_status = NOT_ACTIVE;
			logMessage("-----[t31] Non invio messaggi di ping", taskName(0));
			break;
		default:
			//Se siamo in ACTIVE o NOT_ACTIVE non faccio nulla
			logMessage("-----[t31] Non invio messaggi di ping", taskName(0));
			break;
	}
	logMessage("-----[t19] Rilascio semaforo", taskName(0));
	if(semGive(WIFI_DIAG_SEM) < 0)return E_DEFAUL_ERROR;
	
	// Ritorniamo
	return E_SUCCESS;
	
}

void wifiMain(void){
	
	//aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, wifiDestructor);

	//monitoraggio dei socket con select

	char msg[100] = {'\0'};;
	char log_msg[100] = {'\0'};

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

		//Controllo lo stato del diagnosticsTask
		exit_number status_diag;
		if((status_diag = checkDiag()) != E_SUCCESS){
			logMessage(errorDescription(status_diag), taskName(0));
		}

		// resetto il timeout(viene modificato da select)
		struct timeval select_timeout={.tv_sec=0, .tv_usec=1000};
		// controlla senza bloccare se un socket riceve un msg
		n_ready_conn = select(nfds, &readfds, NULL, NULL, &select_timeout);
		if(n_ready_conn > 0){
			// controllo quali fd sono rimasti in readfds (quelli che hanno ricevuto un msg)
			for(int conn_idx=0; conn_idx<total_conn; conn_idx++){
				if(FD_ISSET(node_conn[conn_idx].sock, &readfds)){
					exit_number status;
					memset(msg, 0, 100);
					if((status = readFromConn(&node_conn[conn_idx], msg, 100)) == E_SUCCESS){
						//debug
						snprintf(log_msg, 100, "Ricevuto messaggio da Rasp id : %i, %s", node_conn[conn_idx].connected_id, msg);
						logMessage(log_msg, taskName(0));
						memset(log_msg, 0, 100);
						int sender_id = node_conn[conn_idx].connected_id;
						if((status = handleInMsgs(msg, sender_id)) == E_CLOSE){
							//se ho ricevuto un messaggio CLOSE dall'host termino il task
							flag_running=false;
						}
						else if(status != E_SUCCESS){
							// Faccio il log dell'errore
							logMessage(errorDescription(status), taskName(0));
						}
					}
					else if(status == E_DISCONNECTION){
						//Se un nodo interrompe la connessione loggo
						memset(log_msg, 0, 100);
						snprintf(log_msg, 100, "Disconnesso Rasp id : %i", node_conn[conn_idx].connected_id);
						logMessage(log_msg, taskName(0));
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
		else if (n_ready_conn == 0){
			logMessage("[t49] Preselection WiFi no msg", taskName(0));
			logMessage("[t26] nessun messaggio ricevuto", taskName(0));
		}
		
		else if(n_ready_conn == -1){
			logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
		}

		//gestisco i messaggi da inviare per conto del controlTask
		tpcp_msg out_control_msg;
		logMessage("[t30] acquisisco semaforo per la coda", taskName(0));
		logMessage("[t47] controllo se sono presenti msg da inviare", taskName(0));		
		ssize_t byte_recevied_control = msgQReceive(OUT_CONTROL_QUEUE, (char*)&out_control_msg, sizeof(tpcp_msg), 1);
		if(byte_recevied_control > 0){
			logMessage("[t46] Preselection, presente msg da inviare", taskName(0));
			logMessage("[t9] sposto messaggio dalla coda globale a quella locale", taskName(0));				
			// debug
			// char msg[100];
			// snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", out_control_msg.command, out_control_msg.sender_id, out_control_msg.recevier_id, out_control_msg.route_id);
			// logMessage(msg, taskName(0));
			exit_number status_control;
			if((status_control = handleOutControlMsg(&out_control_msg)) != E_SUCCESS){
				logMessage(errorDescription(status_control), taskName(0));
			}

		}
		// Nel caso non erano presenti messaggi da inviare per conto del task di controllo
		else if (byte_recevied_control < 0){
			if(strcmp(strerror(errno), "S_objLib_OBJ_TIMEOUT") == 0){
				logMessage("[t45] Preselection no msg da inviare", taskName(0));
				logMessage("[t44] non si devono inviare messaggi", taskName(0));
			}
			else{
				logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
			}
		}
		logMessage("[t37] Conclusione ciclo Task WiFi", taskName(0));
	}


	resetConnections();
	logMessage("Terminato", taskName(0));

}








