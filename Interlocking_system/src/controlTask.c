#include "controlTask.h"
#include "diagnosticsTask.h"

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

    logMessage("[t5] acquisisco semaforo per la coda", taskName(0));
    logMessage("[t28] sposto messaggio dalla coda locale a quella globale", taskName(0));

    if(msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
        return E_DEFAUL_ERROR;
    }
    

    return E_SUCCESS;
}

exit_number forwardNotOk(tpcp_msg* msg, int sender_id){

    logMessage("[t5] acquisisco semaforo per la coda", taskName(0));
    logMessage("[t28] sposto messaggio dalla coda locale a quella globale", taskName(0));

    if(sender_id == RASP_ID){
        memset(msg->command , 0, sizeof(msg->command));
        strcpy(msg->command, "NOT_OK");

        if(current_route->rasp_id_next != TAIL_ID){
            msg->recevier_id = current_route->rasp_id_next;
            msg->sender_id = RASP_ID;
        }

        if(msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
            return E_DEFAUL_ERROR;
        }

        msg->recevier_id = current_route->rasp_id_prev;
        msg->sender_id = RASP_ID;

        if(msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
            return E_DEFAUL_ERROR;
        }
    }
    else{
        if(current_route->rasp_id_next != sender_id){
            if(current_route->rasp_id_next != TAIL_ID){
                msg->recevier_id = current_route->rasp_id_next;
                msg->sender_id = RASP_ID;
                if(msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
                    return E_DEFAUL_ERROR;
                }
            }
        }
        
        if(current_route->rasp_id_prev != sender_id){
            msg->recevier_id = current_route->rasp_id_prev;
            msg->sender_id = RASP_ID;

            if(msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
                return E_DEFAUL_ERROR;
            }
        }

    }

    return E_SUCCESS;
    
}

exit_number handleMsg(tpcp_msg* msg, bool direction, tpcp_status new_middleStatus, tpcp_status new_edgeStatus, char* new_command){

    // direction=true indica che i messaggi in questa fase del tpcp si stanno propagando dalla testa verso la coda della route
    // direction=false il contrario
    // new_command è il nuovo comando che dovrà inoltrare il nodo all'edge di una route (o il primo o l'ultimo in base alla fase del tpcp)
    // i nodi intermedi inoltrano semplicemente il comando precedente

    exit_number status;

    if(direction){
        if(current_route->rasp_id_next != TAIL_ID){
            // debug
            // char log_msg[100];
            // snprintf(log_msg, 100, "handleMsg, command :%s sender :%i recivier:%i route:%i", msg->command, msg->sender_id, msg->recevier_id, msg->route_id);
            // logMessage(log_msg, taskName(0));
            NODE_STATUS = new_middleStatus;
            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0));
            status = forwardMsg(msg, current_route->rasp_id_next, NULL);
        }
        else{
            NODE_STATUS = new_edgeStatus;
            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0));
            status = forwardMsg(msg, current_route->rasp_id_prev, new_command);
        }
    }
    else{
        if(current_route->rasp_id_prev != HOST_ID){
            NODE_STATUS = new_middleStatus;
            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0));
            status = forwardMsg(msg, current_route->rasp_id_prev, NULL);
        }
        else{
            NODE_STATUS = new_edgeStatus;
            if(NODE_STATUS == RESERVED || NODE_STATUS == NOT_RESERVED){
                logMessage("[t4] inoltro il messaggio al next o prev", taskName(0));
                status = forwardMsg(msg, current_route->rasp_id_prev, new_command);
            }
            else{
                logMessage("[t4] inoltro il messaggio al next o prev", taskName(0));
                status = forwardMsg(msg, current_route->rasp_id_next, new_command);
            }
        }
    }

    return status;

}

exit_number handleErrorMsg(tpcp_msg* msg, char* current_status){
    char log_msg[100];
    snprintf(log_msg, 100, "Messaggio Errato per lo stato : %s, command :%s sender :%i recivier:%i route:%i", current_status, msg->command, msg->sender_id, msg->recevier_id, msg->route_id);
    logMessage(log_msg, taskName(0));

    // TODO oltre al log si potrebbero gestire alcuni casi di errore
    return E_SUCCESS;
}

void controlMain(void){
    //aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, controlDestructor);

    tpcp_msg in_msg;
    while(true){
        // Aspetto 15 secondi un eventuale messaggio proveniente da coda wifiTask altrimenti procedo ad eseguire il diagTask      
        ssize_t byte_recevied = msgQReceive(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), TICKS_TO_SECOND * 15);
        if(byte_recevied < 0){
            // Se non è presente un messaggio da wifiTask avvio il task di diagnostica e sospendo il task di controllo
            if(strcmp(strerror(errno), "S_objLib_OBJ_TIMEOUT") == 0){              
                logMessage("[t17] Avvio task di diagnostica", taskName(0));
                DIAGNOSTICS_TID = taskSpawn("diagTask", 50, 0, 20000,(FUNCPTR) diagnosticsMain, 0,0,0,0,0,0,0,0,0,0);
                logMessage("-----Task di diagnostica avviato", taskName(0));
                taskSuspend(0);
                //
                // 
                // In questa parte eseguirà task di diagnostica, al termine il task di diagnostica farà ripartire da qui il flusso del task di controllo
                //
                //
                logMessage("-----Task di controllo ripreso", taskName(0));
                // 
                // TODO: in seguito al resume del task  eseguire implementazione per processare la risposta del successo/insuccesso ping
                //
                // Se la diagnostica ha rilevato un problema di connessione impostare lo stato su PING_FAIL_SAFE
                if (in_ping_fail_safe == true){
                    NODE_STATUS = PING_FAIL_SAFE;
                    logMessage("Stato impostato a PING_FAIL_SAFE", taskName(0));
                }
                // nel caso invece la diagnostica abbia avuto successo non si fa niente e si riparte dall'inizio del ciclo while
                else{                 
                    continue;
                }
            }
            else{
                logMessage(strerror(errno), taskName(0));
            }
        }
        else{
            // Flusso normale di esecuzione del task di controllo, nel caso non si sia avviata la diagnostica
            logMessage("[t27] acquisisco semaforo per la coda", taskName(0));
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
            
            logMessage("[t0] processo il messaggio", taskName(0));

            if( current_route == NULL){
                // TODO gestire risposta quando la route non è presente
                logMessage("[t1] setto lo stato", taskName(0));
                exit_number status;
                if((status = forwardMsg(&in_msg, in_msg.sender_id, "NOT_OK")) != E_SUCCESS){
                    logMessage(errorDescription(status), taskName(0));
                }
                if((status = resetNodeStatus()) != E_SUCCESS){
                    logMessage(errorDescription(status), taskName(0));
                }
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

                //-SENSOR_ON

                // i messaggi negativi potrebbero essere tutti raccolti in unico comando per semplificare la gestione
                // -NOT_OK

            
                //TODO gestire tutti i msg ricevuti in uno stato che non li supporta
                exit_number status = E_SUCCESS,reset_status = E_SUCCESS;
                char log_msg[100];
                switch (NODE_STATUS)
                {
                case NOT_RESERVED:
                    if (strcmp(in_msg.command, "REQ") == 0){
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = handleMsg(&in_msg, true, WAIT_ACK, WAIT_COMMIT, "ACK");
                    }
                    else if (strcmp(in_msg.command, "SENSOR_OFF") == 0){
                        status = handleMsg(&in_msg, false, NOT_RESERVED, NOT_RESERVED, "TRAIN_PASSED");
                        // Resetto la flag che indica l'host corrente
                        if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
                        CURRENT_HOST = -1;
                        if(semGive(WIFI_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
                    }
                    else{
                        status = handleErrorMsg(&in_msg, "NOT_RESERVED");
                    }
                    break;
                case WAIT_ACK:
                    if (strcmp(in_msg.command, "ACK") == 0){
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = handleMsg(&in_msg, false, WAIT_COMMIT, WAIT_AGREE, "COMMIT");
                    }
                    else if (strcmp(in_msg.command, "NOT_OK") == 0){
                        reset_status = resetNodeStatus();
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = forwardNotOk(&in_msg, in_msg.sender_id);
                        
                    }
                    else{
                        status = handleErrorMsg(&in_msg, "WAIT_ACK");
                    }
                    break;
                case WAIT_COMMIT:
                    if (strcmp(in_msg.command, "COMMIT") == 0){
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = handleMsg(&in_msg, true, WAIT_AGREE, RESERVED, "AGREE");
                    }
                    else if (strcmp(in_msg.command, "NOT_OK") == 0){
                        reset_status = resetNodeStatus();
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = forwardNotOk(&in_msg, in_msg.sender_id);
                    }
                    else{
                        status = handleErrorMsg(&in_msg, "WAIT_COMMIT");
                    }
                    break;
                case WAIT_AGREE:
                    if (strcmp(in_msg.command, "AGREE") == 0){
                        logMessage("[t1] setto lo stato", taskName(0));
                        if(NODE_TYPE == TYPE_SWITCH && !IN_POSITION ){
                            //se il nodo è di scambio e non è in posizione avvio il positioning task
                            logMessage("[t16] Non in posizione, avvio il positioning task", taskName(0));
                            POSITIONING_TID = taskSpawn("positioningTask", 50, 0, 20000,(FUNCPTR) positioningMain, 0,0,0,0,0,0,0,0,0,0);
                            taskSuspend(0);
                            //  Controllo se il positioning è avvenuto correttamente
                            if(!IN_POSITION){
                                logMessage("[t15] Setto lo stato MALFUNCTION e inoltro msg ai vicini", taskName(0));
                                NODE_STATUS = MALFUNCTION;
                                status = forwardNotOk(&in_msg, RASP_ID);
                            }
                            else{
                                logMessage("[t13] Positioning avvenuto, setto lo stato RESERVED", taskName(0));
                                NODE_STATUS = RESERVED;
                                status = forwardMsg(&in_msg, current_route->rasp_id_prev, NULL);
                            }
                        }
                        else{
                            status = handleMsg(&in_msg, false, RESERVED, RESERVED, "TRAIN_OK");
                        }
                    }
                    else if (strcmp(in_msg.command, "NOT_OK") == 0){
                        reset_status = resetNodeStatus();
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = forwardNotOk(&in_msg, in_msg.sender_id);
                    }
                    else{
                        status = handleErrorMsg(&in_msg, "WAIT_AGREE");
                    }
                    break;
                case MALFUNCTION:
                    // Se sono nello stato malfunction rispondo con not_ok a qualsiasi messaggio
                    status = forwardNotOk(&in_msg, RASP_ID);
                    break;
                case RESERVED:
                    // if (strcmp(in_msg.command, "SENSOR_ON") == 0){
                    //     if(current_route->rasp_id_next != TAIL_ID){
                    //         status = forwardMsg(&in_msg, current_route->rasp_id_next, NULL);
                    //         reset_status = resetNodeStatus();
                    //     }
                    //     else{
                    //         status = forwardMsg(&in_msg, HOST_ID, "TRAIN_PASSED");
                    //     }
                    if (strcmp(in_msg.command, "SENSOR_ON") == 0){
                        // logMessage("[t1] setto lo stato", taskName(0));
                        status = handleMsg(&in_msg, true, NOT_RESERVED, NOT_RESERVED, "SENSOR_OFF");
                    }
                    else if (strcmp(in_msg.command, "NOT_OK") == 0){
                        reset_status = resetNodeStatus();
                        logMessage("[t1] setto lo stato", taskName(0));
                        status = forwardNotOk(&in_msg, in_msg.sender_id);
                    }
                    else{
                        status = handleErrorMsg(&in_msg, "RESERVED");
                    }
                    break;
                default:
                    memset(log_msg, 0, 100);
                    snprintf(log_msg, 100, "Stato del nodo non riconosciuto : %i", NODE_STATUS);
                    logMessage(log_msg, taskName(0));
                    break;
                }
                if(status != E_SUCCESS){
                    logMessage(errorDescription(status), taskName(0));
                }
                if(reset_status != E_SUCCESS){
                    logMessage(errorDescription(reset_status), taskName(0));
                }
            }
        }
    }
}

void controlDestructor(int sig){
    if(resetNodeStatus() != E_SUCCESS){
        logMessage("Errore nella chiusura del control Task", taskName(0));
    }
    taskDelete(0);
}



exit_number resetNodeStatus(){
    //resetto lo stato del nodo
    NODE_STATUS=NOT_RESERVED;
    if(semTake(WIFI_CONTROL_SEM, WAIT_FOREVER) < 0)return E_DEFAUL_ERROR;
    CURRENT_HOST = -1;
    if(semGive(WIFI_CONTROL_SEM) < 0)return E_DEFAUL_ERROR;
    return E_SUCCESS;
}