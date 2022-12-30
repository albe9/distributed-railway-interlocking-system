/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include <errno.h>
#include "taskLib.h"
#include <arpa/inet.h>
#include "sockLib.h"

#include "logTask.h"



TASK_ID taskId;
char message[20] = "log di prova";

void myRoutine(_Vx_usr_arg_t message){
	logMsg("Messaggio di prova : %s", message, 0, 0, 0, 0, 0);
	logMsg("Task name : %s", taskName(0), 0, 0, 0, 0, 0);
}



void myRoutine4(void){
	int maxTasks = 50;
	TASK_ID id_list[maxTasks];
	TASK_DESC task_info;

	
	taskIdListGet(id_list, maxTasks);
	for(int i=0;i<maxTasks;i++)
	{
		if(taskInfoGet(id_list[i], &task_info) == ERROR){
			printf("\nErrore al task numero : %i\n",i);
		}
		else{
			printf("\ntask : %s\n",task_info.td_name);
		}
	}
	
}

void wifiRoutine(void){
	
	int PORT = 6543;
	char HOST[] = "192.168.1.202";
	
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		perror("Errore ");
	}
	
	serv_addr.sin_addr.s_addr = inet_addr(HOST);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
 
 
	if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		printf("\nConnection Failed \n");
		perror("Errore ");
	}
	
	char* hello = "Hello from client";
	char buffer[1024] = { '\0' };
	send(sock, hello, strlen(hello), 0);
	printf("Hello message sent\n");
	
	valread = read(sock, buffer, 1024);
	printf("Messaggio ricevuto : %s\n", buffer);
	
	// closing the connected socket
	close(client_fd);
	
}

void wifiServerRoutine(void){
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

void startLog(void){
	taskId = taskSpawn("log_init_task", 1, 0, 20000,(FUNCPTR) logInit, 0,0,0,0,0,0,0,0,0,0);
}


void start3(void){
	taskId = taskSpawn("test task", 1, 0, 20000,(FUNCPTR) myRoutine4, 0,0,0,0,0,0,0,0,0,0);
}
void wifi(void){
	taskId = taskSpawn("wifi_task", 1, 0, 20000,(FUNCPTR) wifiRoutine, 0,0,0,0,0,0,0,0,0,0);
}

void wifiServer(void){
	taskId = taskSpawn("wifiServer_task", 1, 0, 20000,(FUNCPTR) wifiServerRoutine, 0,0,0,0,0,0,0,0,0,0);
}

