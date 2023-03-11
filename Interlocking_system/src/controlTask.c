#include "controlTask.h"



void controlMain(void){

    char control_buffer[MAX_CTRL_SIZE];

    while(true){
        ssize_t byte_recevied = msgQReceive(CONTROL_QUEUE, control_buffer, MAX_CTRL_SIZE, WAIT_FOREVER);
        
        logMessage(control_buffer, taskName(0));
        

    }
}
