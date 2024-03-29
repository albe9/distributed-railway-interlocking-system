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
			logMessage(log_msg, taskName(0), 1);
			return E_INVALID_ID;
		}
		else{
			memset(log_msg, 0, 100);
			snprintf(log_msg, 100, "Server id : %i aggiunto correttamente", server_id);
			logMessage(log_msg, taskName(0), 1);
			node_conn[total_conn].connected_id = server_id;
			total_conn++;
			return E_SUCCESS;
		}
	}
	else{
		logMessage("Errore raggiunto numero massimo di socket per questo nodo", taskName(0), 1);
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
	logMessage("Sto aspettando connessioni", taskName(0), 1);


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
			logMessage(log_msg, taskName(0), 1);
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
	logMessage("Nessun rasp ID presente uguale a quello cercato, ritorno un connection* null", taskName(0), 1);
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
	logMessage("Task wifi eliminato",taskName(0), 1);
	taskDelete(0);
}


void resetConnections(){
	
		
	for(int i=0; i< total_conn; i++){
		shutdown(node_conn[i].sock, SHUT_RDWR);
		if (close(node_conn[i].sock) < 0){
			logMessage( errorDescription(E_DEFAUL_ERROR) ,taskName(0), 2);
		}
		node_conn[i] = (const connection){0};
	}
	total_conn = 0;
	
}

void processLogToSend(void){

	logMessage("Avvio task di trasferimento log verso l'host", taskName(0), 1);
	// creo stringa per messaggi di debug
	char feedback_msg[100];
	// Si crea un socket verso l'host
	connection host_conn;
	connectToServer(&host_conn, HOST_IP, LOG_PORT);
	// connectToServer(&host_conn, "192.168.1.202", LOG_PORT);

	// Si sospende il logTask
	if(taskSuspend(LOG_TID) < 0){
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile sospendere il LogTask : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}
	
	
	sendToConn(&host_conn, "Iniziato task di invio log e interrotto il logTask.");

	// Si legge quanto è lungo il file e lo si comunica all'host 
	struct stat log_stat; 
	if(fstat(LOG_FD, &log_stat) < 0){
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile utilizzare fstat sul file di log : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		// Riavviamo il logTask
		if(taskResume(LOG_TID) < 0){
			int err = errno;
			memset(feedback_msg, 0, 100);
			snprintf(feedback_msg, 100, "Impossibile riavviare il LogTask : %s.", strerror(err));
			sendToConn(&host_conn, feedback_msg);
			shutdown(host_conn.sock, SHUT_RDWR);
			close(host_conn.sock);
			exit(-1);
		}
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}
	off_t logSize = log_stat.st_size;
	snprintf(feedback_msg, 100, "File di log di dimensione : [%lld] bytes.", (long long)logSize);
	sendToConn(&host_conn, feedback_msg);


	if(remove("/usr/log/log_tmp_copy.txt") == -1){
		// Non è un problema, vuol dire che non è stato ancora creato
	}

	// Effettuiamo la copia del log.txt e in caso di errori lo comunichiamo all'host
	sendToConn(&host_conn, "Iniziata la copia locale del file di log.");

	int  fd_tmp_log;

	if ((fd_tmp_log = open("/usr/log/log_tmp_copy.txt", O_WRONLY | O_CREAT, 00700)) == -1)
	{
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile creare log_tmp_copy.txt: %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		// Riavviamo il logTask
		if(taskResume(LOG_TID) < 0){
			int err = errno;
			memset(feedback_msg, 0, 100);
			snprintf(feedback_msg, 100, "Impossibile riavviare il LogTask : %s.", strerror(err));
			sendToConn(&host_conn, feedback_msg);
			shutdown(host_conn.sock, SHUT_RDWR);
			close(host_conn.sock);
			exit(-1);
		}
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}
	
    char buffer_to_copy[8192];
    ssize_t bytesRead = 0, bytesWritten = 0;

	if (lseek(LOG_FD, 0, SEEK_SET) == -1) {
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile eseguire lseek su LOG_FD : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		close(fd_tmp_log);
		// Riavviamo il logTask
		if(taskResume(LOG_TID) < 0){
			int err = errno;
			memset(feedback_msg, 0, 100);
			snprintf(feedback_msg, 100, "Impossibile riavviare il LogTask : %s.", strerror(err));
			sendToConn(&host_conn, feedback_msg);
			shutdown(host_conn.sock, SHUT_RDWR);
			close(host_conn.sock);
			exit(-1);
		}
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}

    while ((bytesRead = read(LOG_FD, buffer_to_copy, sizeof(buffer_to_copy))) > 0) {
        bytesWritten = write(fd_tmp_log, buffer_to_copy, bytesRead);
        if (bytesWritten < 0) {
            int err = errno;
			memset(feedback_msg, 0, 100);
			snprintf(feedback_msg, 100, "Errore durante la scrittura su fd_tmp_log : %s.", strerror(err));
			sendToConn(&host_conn, feedback_msg);
			close(fd_tmp_log);
			// Riavviamo il logTask
			if(taskResume(LOG_TID) < 0){
				int err = errno;
				memset(feedback_msg, 0, 100);
				snprintf(feedback_msg, 100, "Impossibile riavviare il LogTask : %s.", strerror(err));
				sendToConn(&host_conn, feedback_msg);
				shutdown(host_conn.sock, SHUT_RDWR);
				close(host_conn.sock);
				exit(-1);
			}
			shutdown(host_conn.sock, SHUT_RDWR);
			close(host_conn.sock);
            exit(-1);
        }
    }

    if (bytesRead < 0) {
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Errore durante la lettura da LOG_FD : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		close(fd_tmp_log);
		// Riavviamo il logTask
		if(taskResume(LOG_TID) < 0){
			int err = errno;
			memset(feedback_msg, 0, 100);
			snprintf(feedback_msg, 100, "Impossibile riavviare il LogTask : %s.", strerror(err));
			sendToConn(&host_conn, feedback_msg);
			shutdown(host_conn.sock, SHUT_RDWR);
			close(host_conn.sock);
			exit(-1);
		}
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
    }

    close(fd_tmp_log);

	// Riavviamo il logTask
	if(taskResume(LOG_TID) < 0){
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile riavviare il LogTask : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}

	sendToConn(&host_conn, "Riavviato il logTask, inizio invio dei log [START_SENDING].");

	// Attendiamo così da essere sicuri che lo script python sia pronto a ricevere i log
	taskDelay(5);

	// Apriamo la copia dei log per spedirli
	if ((fd_tmp_log = open("/usr/log/log_tmp_copy.txt", O_RDONLY , 00700)) < 0){
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile aprire fd_tmp_log : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}


	char buffer_to_send[SO_SNDBUF];
	while((bytesRead = read(fd_tmp_log, buffer_to_send, sizeof(buffer_to_send))) > 0) {
        sendToConn(&host_conn, buffer_to_send);
    }

	if(bytesRead < 0){
		int err = errno;
		memset(feedback_msg, 0, 100);
		snprintf(feedback_msg, 100, "Impossibile leggere da fd_tmp_log : %s.", strerror(err));
		sendToConn(&host_conn, feedback_msg);
		shutdown(host_conn.sock, SHUT_RDWR);
		close(host_conn.sock);
		exit(-1);
	}


	// comunico all'host che l'invio ha avuto successo
	sendToConn(&host_conn, "[END_WITH_SUCCESS]");
	shutdown(host_conn.sock, SHUT_RDWR);
	close(host_conn.sock);

	logMessage("Inviato con successo il log all'host", taskName(0), 1);

}

void sendLogToHost(void){
	SEND_LOG_TID = taskCreate("sendLogTask", PRI_0, 0, 100000,(FUNCPTR) processLogToSend, 0,0,0,0,0,0,0,0,0,0);
	taskCpuAffinitySet(SEND_LOG_TID, 1 << 2);
    taskActivate(SEND_LOG_TID);
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
			logMessage(errorDescription(status), taskName(0), 2);
		}
    }
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
	        // printf("%i,   %i\n", strcmp(command_type, "PING_REQ"), strcmp(command_type, "PING_ACK"));
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
		sprintf(tmp_ping_msg, "Ricevuto comando PING_REQ da nodo %i", sender_id);
		logMessage(tmp_ping_msg, taskName(0), 1);
		memset(tmp_ping_msg, 0, 100);		
		//Rispondo al ping segnalando di essere un nodo attivo
		sendToConn(conn_ping, "PING_ACK;.");
		logMessage("[t48] Preselection WiFi msg diagnostica", taskName(0), 0);
		sprintf(tmp_ping_msg, "[t11] Inviato comando PING_ACK al nodo %i", sender_id);
		logMessage(tmp_ping_msg, taskName(0), 0);
		memset(tmp_ping_msg, 0, 100);
	}
	else if (strcmp(command_type, "PING_ACK") == 0){
		char tmp_ping_msg_2[100];
		logMessage("[t48] Preselection WiFi msg diagnostica", taskName(0), 0);
		sprintf(tmp_ping_msg_2, "[t11] Ricevuto comando PING_ACK da nodo %i", sender_id);
		logMessage(tmp_ping_msg_2, taskName(0), 0);
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
	else if (strcmp(command_type, "SENSOR_ON") == 0){
		if(semTake(WIFI_CONTROL_SEM, WAIT_FOREVER) < 0)return E_DEFAUL_ERROR;
		logMessage("Ricevuto messaggio simulato di SENSOR_ON ", taskName(0), 1);
		sensor_on_detected = true;
		if(semGive(WIFI_CONTROL_SEM) < 0)return E_DEFAUL_ERROR;
	}
	else{
		//gestiamo i messaggi relativi al TPCP
		tpcp_msg in_msg;
		
		//debug
		// logMessage(in_msg.command, taskName(0));


		int msg_host = -1;
		if(sscanf(msg_data,"%i;%i", &msg_host, &in_msg.route_id) != 2)return(E_PARSING);
		//acquisisco il semaforo per accedere alla variabile globale (è condivisa con il task di controllo)
		if(semTake(WIFI_CONTROL_SEM, WAIT_FOREVER) < 0)return E_DEFAUL_ERROR;
		//se non ci sono host correnti lo setto
		if(CURRENT_HOST == -1){
			CURRENT_HOST = msg_host;
		}

		if(CURRENT_HOST != msg_host){
			in_msg.recevier_id = sender_id;
			in_msg.sender_id = RASP_ID;
			strcpy(in_msg.command, "NOT_OK");
			in_msg.host_id = msg_host;
			handleOutControlMsg(&in_msg, true);
		}
		else{
			//passo i dati al task di controllo
			in_msg.recevier_id=RASP_ID;
			in_msg.sender_id=sender_id;
			strcpy(in_msg.command, command_type);
			in_msg.host_id = CURRENT_HOST;
			logMessage("[t29] Preselection WiFi msg controllo", taskName(0), 0);
			logMessage("[t8] filtraggio messaggio concluso", taskName(0), 0);
			logMessage("[t6] acquisisco il semaforo per la coda", taskName(0), 0);
			taskPrioritySet(0, PRI_3);
			logMessage("[t40] sposto messaggio dalla coda locale a quella globale", taskName(0), 0);
			logMessage("[t23] rilascio semaforo", taskName(0), 0);
			taskPrioritySet(0, PRI_2);
			msgQSend(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
		}	
		if(semGive(WIFI_CONTROL_SEM) < 0)return E_DEFAUL_ERROR;
	}


	return E_SUCCESS;
}

exit_number handleOutControlMsg(tpcp_msg* out_control_msg, bool flag_log_not_ok){

	//debug
	char log_msg[100];
	snprintf(log_msg, 100, "Prima di inoltrare il msg, command :%s sender :%i recivier:%i route:%i", out_control_msg->command, out_control_msg->sender_id, out_control_msg->recevier_id, out_control_msg->route_id);
	logMessage(log_msg, taskName(0), 1);

	for(int node_idx=0; node_idx<total_conn; node_idx++){
		if(node_conn[node_idx].connected_id == out_control_msg->recevier_id){
			char msg[100];
			// Riformatto il pacchetto seguendo la sintassi definita in handleInSingleMsg
			snprintf(msg, 100,"%s;%i;%i.", out_control_msg->command, out_control_msg->host_id, out_control_msg->route_id);
			sendToConn(&node_conn[node_idx], msg);
			if (flag_log_not_ok){
				logMessage("[t48] Preselection", taskName(0), 0);
				logMessage("[t11] Messaggio NOT_OK inviato", taskName(0), 0);
			}
			else{
				logMessage("[t10] Messaggio inviato", taskName(0), 0);
			}


			return E_SUCCESS;
		}
	}

	//Se non trovo nessun nodo corrispondente significa che c'è stato un errore
	return E_NODE_NOTFOUND;

}

exit_number checkDiag(){
	if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0)return E_DEFAUL_ERROR;
	logMessage("[t18] acquisisco semaforo", taskName(0), 0);
	//taskPrioritySet(0, PRI_2);
	logMessage("[t32] controllo area di memoria", taskName(0), 0);
	switch (ping_status)
	{
		case STARTING:
			// Inviamo un messaggio PING_REQ a tutti i nodi vicini
			for(int node_idx=0; node_idx<total_conn; node_idx++){
				sendToConn(&node_conn[node_idx], "PING_REQ;.");
			}
			logMessage("[t31] Inviato messaggi PING_REQ a tutti i vicini", taskName(0), 0);
			// Cambiamo lo stato del ping
			ping_status = ACTIVE;
			break;
		case ENDING:
			// Azzeriamo il contatore delle risposte al ping e indichiamo che la procedura di ping è disattivata
			ping_answers = 0;
			ping_status = NOT_ACTIVE;
			logMessage("[t31] Non invio messaggi di ping", taskName(0), 0);
			break;
		default:
			//Se siamo in ACTIVE o NOT_ACTIVE non faccio nulla
			logMessage("[t31] Non invio messaggi di ping", taskName(0), 0);
			break;
	}
	logMessage("[t19] Rilascio semaforo", taskName(0), 0);
	if(semGive(WIFI_DIAG_SEM) < 0)return E_DEFAUL_ERROR;
	//taskPrioritySet(0, PRI_1);
	
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
		logMessage("[t54] inizio ciclo Task WiFi", taskName(0), 0);   
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
			logMessage(errorDescription(status_diag), taskName(0), 2);
		}

		// resetto il timeout(viene modificato da select)
		// TODO Questi micro secondi non sono un multiplo di 1 tick
		//		quindi aspettare 1 millisecondo non è possibile, ne aspetta 16/17
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
						logMessage(log_msg, taskName(0), 1);
						memset(log_msg, 0, 100);
						int sender_id = node_conn[conn_idx].connected_id;
						if((status = handleInMsgs(msg, sender_id)) == E_CLOSE){
							//se ho ricevuto un messaggio CLOSE dall'host termino il task
							flag_running=false;
						}
						else if(status != E_SUCCESS){
							// Faccio il log dell'errore
							logMessage(errorDescription(status), taskName(0), 2);
						}
					}
					else if(status == E_DISCONNECTION){
						//Se un nodo interrompe la connessione loggo
						memset(log_msg, 0, 100);
						snprintf(log_msg, 100, "Disconnesso Rasp id : %i", node_conn[conn_idx].connected_id);
						logMessage(log_msg, taskName(0), 1);
						// Setto il colore del led
						changeLedColor(FAIL_COL);
						// chiudo il socket relativo e rimuovo la sua connesione dall'array delle connessioni attive
						shutdown(node_conn[conn_idx].sock, SHUT_RDWR);
						if (close(node_conn[conn_idx].sock) < 0){
							logMessage("Impossibile chiudere il socket",taskName(0), 2);
						}
						node_conn[conn_idx] = (const connection){0};
						total_conn -= 1;
						for(int j=conn_idx; j<total_conn;j++){
							node_conn[j] = node_conn[j+1];
						}
					}
				}
			}
			//Dopo aver controllato tutti i socket non ho più msg da gestire e si procede
			logMessage("[t49] Preselection WiFi no msg", taskName(0), 0);
			logMessage("[t26] Non ci sono messaggi da gestire", taskName(0), 0);
		}
		else if (n_ready_conn == 0){
			logMessage("[t49] Preselection WiFi no msg", taskName(0), 0);
			logMessage("[t26] nessun messaggio ricevuto", taskName(0), 0);
		}
		
		else if(n_ready_conn == -1){
			logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
		}

		//gestisco i messaggi da inviare per conto del controlTask
		tpcp_msg out_control_msg;
		ssize_t byte_recevied_control = msgQReceive(OUT_CONTROL_QUEUE, (char*)&out_control_msg, sizeof(tpcp_msg), 1);
		logMessage("[t78] attesa acquisizione msg completata", taskName(0), 0);
		logMessage("[t30] acquisisco semaforo per la coda", taskName(0), 0);
		taskPrioritySet(0, PRI_3);
		logMessage("[t47] controllo se sono presenti msg da inviare", taskName(0), 0);	
		if(byte_recevied_control > 0){
			logMessage("[t46] Preselection, presente msg da inviare", taskName(0), 0);
			logMessage("[t9] sposto messaggio dalla coda globale a quella locale", taskName(0), 0);
			taskPrioritySet(0, PRI_2);				
			// debug
			// char msg[100];
			// snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", out_control_msg.command, out_control_msg.sender_id, out_control_msg.recevier_id, out_control_msg.route_id);
			// logMessage(msg, taskName(0));
			exit_number status_control;
			if((status_control = handleOutControlMsg(&out_control_msg, false)) != E_SUCCESS){
				logMessage(errorDescription(status_control), taskName(0), 2);
			}

		}
		// Nel caso non erano presenti messaggi da inviare per conto del task di controllo
		else if (byte_recevied_control < 0){
			if(strcmp(strerror(errno), "S_objLib_OBJ_TIMEOUT") == 0){
				logMessage("[t45] Preselection no msg da inviare", taskName(0), 0);
				logMessage("[t44] non si devono inviare messaggi", taskName(0), 0);
				taskPrioritySet(0, PRI_2);
			}
			else{
				logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
			}
		}
		logMessage("[t37] Conclusione ciclo Task WiFi", taskName(0), 0);
		taskDelay(8);
		logMessage("[t77] task suspend 8 tick terminato", taskName(0), 0);
	}


	resetConnections();
	logMessage("Terminato", taskName(0), 1);

}








