#include "controlTask.h"

static tpcp_status NODE_STATUS=NOT_RESERVED;
static int HOST_ID = 0;    //per adesso gestiamo un solo host
static route* current_route = NULL;

// TODO cercare api per capire cosa ritorna semtake e semgive

exit_number forwardMsg(tpcp_msg* msg, int receiver_id, char* command){

    // debug
    // char log_msg[100];
    // snprintf(log_msg, 100, "forwardMsg, command :%s sender :%i recivier:%i route:%i", msg->command, msg->sender_id, msg->recevier_id, msg->route_id);
    // logMessage(log_msg, taskName(0));

    if(command != NULL){
        memset(msg->command , 0, sizeof(msg->command));
        strcpy(msg->command, command);
    }

    msg->recevier_id = receiver_id;
    msg->sender_id = RASP_ID;

    logMessage("[t4] inoltro il messaggio al next o prev", taskName(0));
    logMessage("[t5] acquisisco semaforo per la coda", taskName(0));
    msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
    logMessage("[t28] sposto messaggio dalla coda locale a quella globale", taskName(0));

    return E_SUCCESS;
}

exit_number handleMsg(tpcp_msg* msg, bool direction, tpcp_status new_middleStatus, tpcp_status new_edgeStatus, char* new_command){

    // direction=true indica che i messaggi in questa fase del tpcp si stanno propagando dalla testa verso la coda della route
    // direction=false il contrario
    // new_command è il nuovo comando che dovrà inoltrare il nodo all'edge di una route (o il primo o l'ultimo in base alla fase del tpcp)
    // i nodi intermedi inoltrano semplicemente il comando precedente
    logMessage("[t0] processo il messaggio", taskName(0));

    exit_number status;

    if(direction){
        if(current_route->rasp_id_next != TAIL_ID){
            // debug
            // char log_msg[100];
            // snprintf(log_msg, 100, "handleMsg, command :%s sender :%i recivier:%i route:%i", msg->command, msg->sender_id, msg->recevier_id, msg->route_id);
            // logMessage(log_msg, taskName(0));
            NODE_STATUS = new_middleStatus;
            logMessage("[t1] setto lo stato", taskName(0));
            forwardMsg(msg, current_route->rasp_id_next, NULL);
        }
        else{
            NODE_STATUS = new_edgeStatus;
            logMessage("[t1] setto lo stato", taskName(0));
            forwardMsg(msg, current_route->rasp_id_prev, new_command);
        }
    }
    else{
        if(current_route->rasp_id_prev != HOST_ID){
            NODE_STATUS = new_middleStatus;
            logMessage("[t1] setto lo stato", taskName(0));
            forwardMsg(msg, current_route->rasp_id_prev, NULL);
        }
        else{
            NODE_STATUS = new_edgeStatus;
            logMessage("[t1] setto lo stato", taskName(0));
            if(NODE_STATUS == TRAIN_IN_TRANSITION){
                forwardMsg(msg, current_route->rasp_id_prev, new_command);
            }
            else{
                forwardMsg(msg, current_route->rasp_id_next, new_command);
            }
        }
    }

    return E_SUCCESS;

}


void controlMain(void){

    tpcp_msg in_msg;

    while(true){
        logMessage("[t27] acquisisco semaforo per la coda", taskName(0));
        ssize_t byte_recevied = msgQReceive(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER);
        logMessage("[t7] sposto semaforo dalla coda globale a quella locale", taskName(0));


        char msg[100];
        snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", in_msg.command, in_msg.sender_id, in_msg.recevier_id, in_msg.route_id);
        logMessage(msg, taskName(0));


        // resetto la rotta precedente e verifico che il nodo appartenga a quella descritta nel messaggio
        current_route = NULL;

        for(int route_idx=0; route_idx<route_count; route_idx++){
            if(node_routes[route_idx].route_id == in_msg.route_id){
                current_route = &node_routes[route_idx];
                break;
            }
        }

        if( current_route == NULL){
            // TODO gestire risposta quando la route non è presente
            forwardMsg(&in_msg, in_msg.sender_id, "NOT_OK");
            resetNodeStatus();
        }
        else{

            //Gestisco i messaggi ricevuti dal task wifi

            
                // campo command:

                // -REQ
                // -WAIT_ACK
                // -ACK

                // -WAIT_COMMIT
                // -COMMIT

                // -WAIT_AGREE
                // -AGREE

                // i messaggi negativi potrebbero essere tutti raccolti in unico comando per semplificare la gestione
                // -NOT_OK

            
            //TODO gestire tutti i msg ricevuti in uno stato che non li supporta

            switch (NODE_STATUS)
            {
            case NOT_RESERVED:
                if (strcmp(in_msg.command, "REQ") == 0){
                    handleMsg(&in_msg, true, WAIT_ACK, WAIT_COMMIT, "ACK");
                }
                else{
                    // TODO gestire messaggi sbagliati
                }
                break;
            case WAIT_ACK:
                if (strcmp(in_msg.command, "ACK") == 0){
                    handleMsg(&in_msg, false, WAIT_COMMIT, WAIT_AGREE, "COMMIT");
                }
                else if (strcmp(in_msg.command, "NOT_OK") == 0){
                    resetNodeStatus();
                    forwardMsg(&in_msg, current_route->rasp_id_prev, NULL);
                    
                }
                else{
                    // TODO gestire messaggi sbagliati
                }
                break;
            case WAIT_COMMIT:
                if (strcmp(in_msg.command, "COMMIT") == 0){
                    handleMsg(&in_msg, true, WAIT_AGREE, RESERVED, "AGREE");
                }
                else if (strcmp(in_msg.command, "NOT_OK") == 0){
                    resetNodeStatus();
                    if(current_route->rasp_id_next != TAIL_ID){
                        forwardMsg(&in_msg, current_route->rasp_id_next, NULL);
                    }
                    else{
                        forwardMsg(&in_msg, HOST_ID, NULL);
                    }
                }
                else{
                    // TODO gestire messaggi sbagliati
                }
                break;
            case WAIT_AGREE:
                if (strcmp(in_msg.command, "AGREE") == 0){
                    if(NODE_TYPE == TYPE_SWITCH){
                        // TODO verificare il positioning
                    }
                    handleMsg(&in_msg, false, RESERVED, TRAIN_IN_TRANSITION, "TRAIN_OK");
                }
                else if (strcmp(in_msg.command, "NOT_OK") == 0){
                    resetNodeStatus();
                    forwardMsg(&in_msg, current_route->rasp_id_prev, NULL);
                }
                else{
                    // TODO gestire messaggi sbagliati
                }
                break;
            case RESERVED:
                
                break;
            default:
                break;
            }
        }
    }
}

void hookControlDelete(_Vx_TASK_ID tid){
    if(strcmp(taskName(tid), "controlTask") == 0){
        if(resetNodeStatus() != E_SUCCESS){
            logMessage("Errore nella chiusura del control Task", taskName(0));
        }
    }
}

exit_number resetNodeStatus(){
    //resetto lo stato del nodo
    logMessage("[t0] processo il messaggio", taskName(0));
    NODE_STATUS=NOT_RESERVED;
    semTake(GLOBAL_SEM, WAIT_FOREVER);
    CURRENT_HOST = -1;
    semGive(GLOBAL_SEM);
    logMessage("[t1] setto lo stato", taskName(0));
    return E_SUCCESS;
}