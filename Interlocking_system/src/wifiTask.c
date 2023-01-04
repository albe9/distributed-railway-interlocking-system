/*
 * wifiTask.c
 *
 *  Created on: Jan 4, 2023
 *      Author: alber
 */


#include "wifiTask.h"

int HOST_PORT = 6543;
char HOST_IP[] = "192.168.1.35";
int sock_server = 0, fd_server = 0;

void addServer(char* server_ip, int server_port){
	
	
	struct sockaddr_in serv_addr;
	
	if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("\nErrore nella creazione del socket client function : ");
	}
	
	serv_addr.sin_addr.s_addr = inet_addr(server_ip);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);
 
 
	if ((fd_server = connect(sock_server, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		perror("\nErrore connessione fallita client function : ");
	}
	
	
}

void sendToServer(char *msg){
	
	send(sock_server, msg, strlen(msg), 0);
}

void readFromServer(char* buffer, ssize_t buf_size){
	
	ssize_t valread;
	valread = read(sock_server, buffer, buf_size);
	printf("Messaggio ricevuto : %s\n", buffer);
}

void removeServer(void){
	// closing the connected socket
	close(fd_server);
}
/*
void setServer(void){
	int sock, new_sock, addrlen;
	struct sockaddr_in server, client;
	
	
	
	//Create socket
	
	if ((sock = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		printf("\nImpossibile creare il socket\n");
		perror("Errore ");
	}
	
	//Set sock option
	
	int reuse_addr = 1;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse_addr,sizeof(int)) < 0)
	{
		perror("Setsockopt");
	}
	
		
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 6543 );
		
	//Bind
	
	if( bind(sock,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("Errore durante il Bind");
	}
	printf("Bind eseguito");
	
	//listen for connection
	listen(sock , 3);
	//Accept and incoming connection
	printf("\nIn attesa di connessioni\n");
	addrlen = sizeof(struct sockaddr_in);
	if ((new_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&addrlen)) < 0)
	{
		perror("accept failed");
	}
	
	printf("\nConnessione accettata\n");
	
	char host_addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client.sin_addr), host_addr, INET_ADDRSTRLEN);
	printf("\n%s\n",host_addr);
	
	close(new_sock);
	shutdown(sock, SHUT_RDWR);
	if (close(sock) < 0){
		perror("Errore chiusura server socket");
	}
	printf("\nFine connessione\n");
}
*/

void wifiMain(void){
	
	addServer(HOST_IP, HOST_PORT);
	sendToServer("Messaggio di prova verso l'host");
	char buffer[1024] = {"\0"};
	readFromServer(buffer, 1024);
}







