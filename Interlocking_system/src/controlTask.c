#include "controlTask.h"
#define MAX_LOG_SIZE 1024
#define MAX_LOG_BUFF 10

char control_buffer[MAX_LOG_SIZE];

void controlMain(void){
    while(true){
        int byte_recevied = msgQReceive(LOG_QUEUE, control_buffer, MAX_LOG_SIZE, WAIT_FOREVER);


    }
}
