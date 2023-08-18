#include "controlTask.h"
#include "diagnosticsTask.h"

static tpcp_status NODE_STATUS=NOT_RESERVED;
static int HOST_ID = 0;    //per adesso gestiamo un solo host
static route* current_route = NULL;

// TODO cercare api per capire cosa ritorna semtake e semgive

exit_number forwardMsg(tpcp_msg* msg, int receiver_id, char* command, bool log_transitions){

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

    if(log_transitions){
        logMessage("[t5] acquisisco semaforo per la coda", taskName(0), 0);
        logMessage("[t41] Scrivo il messaggio", taskName(0), 0);
        logMessage("[t28] Rilascio il semaforo e abbasso la priorità", taskName(0), 0);
    }

    if(msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
        return E_DEFAUL_ERROR;
    }
    

    return E_SUCCESS;
}

exit_number forwardNotOk(tpcp_msg* msg, int sender_id){

    logMessage("[t5] acquisisco semaforo per la coda", taskName(0), 0);
    logMessage("[t41] Scrivo il messaggio", taskName(0), 0);
    logMessage("[t28] Rilascio il semaforo e abbasso la priorità", taskName(0), 0);

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
            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
            status = forwardMsg(msg, current_route->rasp_id_next, NULL, true);
        }
        else{
            NODE_STATUS = new_edgeStatus;
            if(NODE_STATUS == RESERVED)logMessage("[t12] setto lo stato RESERVED", taskName(0), 0);
            else logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
            status = forwardMsg(msg, current_route->rasp_id_prev, new_command, true);
        }
    }
    else{
        if(current_route->rasp_id_prev != HOST_ID){
            NODE_STATUS = new_middleStatus;
            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
            status = forwardMsg(msg, current_route->rasp_id_prev, NULL, true);
        }
        else{
            NODE_STATUS = new_edgeStatus;
            if(NODE_STATUS == RESERVED)logMessage("[t12] setto lo stato RESERVED", taskName(0), 0);
            else logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
            if(NODE_STATUS == RESERVED || NODE_STATUS == NOT_RESERVED){
    
                status = forwardMsg(msg, current_route->rasp_id_prev, new_command, true);
            }
            else{
    
                status = forwardMsg(msg, current_route->rasp_id_next, new_command, true);
            }
        }
    }

    return status;

}

exit_number handleErrorMsg(tpcp_msg* msg, char* current_status){
    char log_msg[100];
    snprintf(log_msg, 100, "Messaggio Errato per lo stato : %s, command :%s sender :%i recivier:%i route:%i", current_status, msg->command, msg->sender_id, msg->recevier_id, msg->route_id);
    logMessage(log_msg, taskName(0), 1);

    // TODO oltre al log si potrebbero gestire alcuni casi di errore
    return E_SUCCESS;
}

exit_number startDiagn(){
    // Se non è presente un messaggio da wifiTask avvio il task di diagnostica
    if(strcmp(strerror(errno), "S_objLib_OBJ_TIMEOUT") == 0){              
        logMessage("[t17] Avvio task di diagnostica", taskName(0), 0);
        DIAGNOSTICS_TID = taskSpawn("diagTask", 50, 0, 20000,(FUNCPTR) diagnosticsMain, 0,0,0,0,0,0,0,0,0,0);
        logMessage("-----Task di diagnostica avviato", taskName(0), 1);
        return E_SUCCESS;
    }
    else{
        return E_DEFAUL_ERROR;
    }
}

void controlMain(void){
    //aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, controlDestructor);

    // Definiamo un timer per poter far partire la diagnostica secondo le specifiche (Es ogni 15 secondi senza ricezioni di msg)
    clock_t startDiagnTime, currentDiagnTime, 
            startSensorOffTime, currentSensorOffTime;
    double elapsedDiagnTimer, elapsedSensorOffTimer;

    bool flagDiagnOn = false;
    startDiagnTime = tickGet();
    tpcp_msg in_msg;
    tpcp_msg sensors_msg;
    while(true){    

        // Controllo SensorOn, da modificare per passare agli switch fisici
        if(NODE_STATUS == RESERVED){
            if(semTake(WIFI_CONTROL_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
            if(sensor_on_detected){
                logMessage("[t56] preselection", taskName(0), 0);
                logMessage("[t2] sensorOn", taskName(0), 0);
                logMessage("SensorOn abilitato, setto lo stato TRAIN_IN_TRANSITION", taskName(0), 1);
                // Setto lo stato, aggiorno il timer e resetto la flag in comune con il task wifi
                NODE_STATUS = TRAIN_IN_TRANSITION;
                startSensorOffTime = tickGet();
                sensor_on_detected = false;
            }
            else{
                logMessage("[t73] preselection", taskName(0), 0);
                logMessage("[t74] non avviene il sensorOn", taskName(0), 0);
            }
            if(semGive(WIFI_CONTROL_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
        }
        else{
            logMessage("[t73] preselection", taskName(0), 0);
            logMessage("[t74] non avviene il sensorOn", taskName(0), 0);
        }
        // Controllo SensorOff
        // Se sono nello stato SensoOn controllo quanto tempo è passato per effettuare il sensorOff
        if(NODE_STATUS == TRAIN_IN_TRANSITION){
            currentSensorOffTime = tickGet();
            elapsedSensorOffTimer = (double)(currentSensorOffTime - startSensorOffTime)/TICKS_TO_SECOND;
            if(elapsedSensorOffTimer >= 2){
                logMessage("[t57] Preselection", taskName(0), 0);
                logMessage("[t3] SensorOff e setto lo stato NOT_RESERVED", taskName(0), 0);
                sensors_msg.route_id = current_route->route_id;
                sensors_msg.host_id = CURRENT_HOST;

                if(current_route->rasp_id_next != TAIL_ID){
                    forwardMsg(&sensors_msg, current_route->rasp_id_next, "SENSOR_ON", false);
                }
                else{
                    forwardMsg(&sensors_msg, current_route->rasp_id_prev, "SENSOR_OFF", false);
                }
                resetNodeStatus();
            }
            else{
                logMessage("[t53] Preselection", taskName(0), 0);
                logMessage("[t59] Non avviene sensorOff", taskName(0), 0);
            }
        }
        else{
            logMessage("[t53] Preselection", taskName(0), 0);
            logMessage("[t59] Non avviene sensorOff", taskName(0), 0);
        }
        
        // Controllo diagnostica
        if(flagDiagnOn){
            logMessage("[t60] preselection", taskName(0), 0);
            logMessage("[t63] acquisisco il semaforo", taskName(0), 0);
            if(semTake(CONTROL_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
            logMessage("[t68] verifico se diag terminata", taskName(0), 0);
            if(diag_ended){
                logMessage("[t65] preseletion", taskName(0), 0);
                logMessage("[t67] diag terminata", taskName(0), 0);
                if(diag_success){
                    logMessage("[t69] preseletion", taskName(0), 0);
                    logMessage("[t71] diag terminata con successo", taskName(0), 0);
                }
                else{
                    logMessage("[t70] preseletion", taskName(0), 0);
                    logMessage("[t72] diag termina con errore", taskName(0), 0);
                    // TODO gestire stato PING-FAIL-SAFE
                    NODE_STATUS = PING_FAIL_SAFE;
                }
                diag_ended = false;
                diag_success = false;
                flagDiagnOn = false;
                // resetto il timer per la diagnostica (l'ho appena eseguita)
                startDiagnTime = tickGet();
            }
            else{
                logMessage("[t64] preseletion", taskName(0), 0);
                logMessage("[t66] diagnostica da terminare, rilascio sem", taskName(0), 0);
            }
            if(semGive(CONTROL_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
            
        }
        else{
            logMessage("[t61] preselection", taskName(0), 0);
            logMessage("[t62] diag non spawnata", taskName(0), 0);
        }

        // Controllo messaggi da wifiTask
        ssize_t byte_recevied = msgQReceive(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), 1);
        logMessage("[t27] acquisisco semaforo per la coda", taskName(0), 0);
        logMessage("[t39] controllo se presente un messaggio", taskName(0), 0); 

        
        if(byte_recevied < 0){
            logMessage("[t42] Preselezione", taskName(0), 0);
            logMessage("[t38] non presente un msg", taskName(0), 0);
            if(!flagDiagnOn){
                // Calcolo per quanto tempo (in secondi) non ho ricevuto messaggi, se maggiore di DIAG_TIME avvio la diagnostica
                currentDiagnTime = tickGet();
                elapsedDiagnTimer = (double)(currentDiagnTime - startDiagnTime)/TICKS_TO_SECOND;
                if(elapsedDiagnTimer >= DIAG_TIME){
                    logMessage("[t50] Preselezione", taskName(0), 0);
                    exit_number status;
                    if((status = startDiagn()) != E_SUCCESS){
                        logMessage(errorDescription(status), taskName(0), 2);
                    }
                    flagDiagnOn = true;
                }
                else{
                    logMessage("[t51] Preselezione", taskName(0), 0);
                    logMessage("[t52] Ritorno in idle", taskName(0), 0);
                }
            }
            else{
                logMessage("[t51] Preselezione", taskName(0), 0);
                logMessage("[t52] Ritorno in idle", taskName(0), 0);
            }
        }
        else{
            if (NODE_STATUS == NOT_RESERVED && strcmp(in_msg.command, "SENSOR_OFF") == 0){
                //poichè è un messaggio che simula un sensore, faccio finta di non aver ricevuto nessun messaggio nei log e di tornare in idle
                logMessage("[t42] Preselezione", taskName(0), 0);
                logMessage("[t38] nessun msg presente", taskName(0), 0);
                logMessage("[t51] Preselezione", taskName(0), 0);
                logMessage("[t52] controlTask torna in idle", taskName(0), 0);

                logMessage("Inoltro il SensorOff", taskName(0), 1);
                forwardMsg(&in_msg, current_route->rasp_id_prev, NULL, false);
                // Resetto la flag che indica l'host corrente
                resetNodeStatus();
            }
            else{ 
                // Flusso normale di esecuzione del task di controllo
                logMessage("[t43] Preselezione", taskName(0), 0);
                logMessage("[t7] presente un msg", taskName(0), 0);
                char msg[100];
                snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", in_msg.command, in_msg.sender_id, in_msg.recevier_id, in_msg.route_id);
                logMessage(msg, taskName(0), 1);


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
                    logMessage("[t0] Preselection", taskName(0), 0);
                    logMessage("[t1] setto lo stato", taskName(0), 0);
                    logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
                    exit_number status;
                    if((status = forwardMsg(&in_msg, in_msg.sender_id, "NOT_OK", true)) != E_SUCCESS){
                        logMessage(errorDescription(status), taskName(0), 2);
                    }
                    if((status = resetNodeStatus()) != E_SUCCESS){
                        logMessage(errorDescription(status), taskName(0), 2);
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
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato WAIT_ACK/WAIT_COMMIT", taskName(0), 0);
                            status = handleMsg(&in_msg, true, WAIT_ACK, WAIT_COMMIT, "ACK");
                        }
                        else{
                            status = handleErrorMsg(&in_msg, "NOT_RESERVED");
                        }
                        break;
                    case WAIT_ACK:
                        if (strcmp(in_msg.command, "ACK") == 0){
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato WAIT_COMMIT/WAIT_AGREE", taskName(0), 0);
                            status = handleMsg(&in_msg, false, WAIT_COMMIT, WAIT_AGREE, "COMMIT");
                        }
                        else if (strcmp(in_msg.command, "NOT_OK") == 0){
                            reset_status = resetNodeStatus();
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato NOT_RESERVED", taskName(0), 0);
                            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
                            status = forwardNotOk(&in_msg, in_msg.sender_id);
                            
                        }
                        else{
                            status = handleErrorMsg(&in_msg, "WAIT_ACK");
                        }
                        break;
                    case WAIT_COMMIT:
                        if (strcmp(in_msg.command, "COMMIT") == 0){
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato WAIT_AGREE/RESERVED", taskName(0), 0);
                            status = handleMsg(&in_msg, true, WAIT_AGREE, RESERVED, "AGREE");
                        }
                        else if (strcmp(in_msg.command, "NOT_OK") == 0){
                            reset_status = resetNodeStatus();
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato NOT_RESERVED", taskName(0), 0);
                            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
                            status = forwardNotOk(&in_msg, in_msg.sender_id);
                        }
                        else{
                            status = handleErrorMsg(&in_msg, "WAIT_COMMIT");
                        }
                        break;
                    case WAIT_AGREE:
                        if (strcmp(in_msg.command, "AGREE") == 0){
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato", taskName(0), 0);
                            if(NODE_TYPE == TYPE_SWITCH && !IN_POSITION ){
                                //se il nodo è di scambio e non è in posizione avvio il positioning task
                                logMessage("[t16] Non in posizione, avvio il positioning task", taskName(0), 0);
                                POSITIONING_TID = taskSpawn("positioningTask", 50, 0, 20000,(FUNCPTR) positioningMain, 0,0,0,0,0,0,0,0,0,0);
                                taskSuspend(0);
                                //  Controllo se il positioning è avvenuto correttamente
                                if(!IN_POSITION){
                                    logMessage("[t13] Setto lo stato MALFUNCTION e inoltro msg ai vicini", taskName(0), 0);
                                    NODE_STATUS = MALFUNCTION;
                                    status = forwardNotOk(&in_msg, RASP_ID);
                                }
                                else{
                                    logMessage("[t15] Positioning avvenuto, setto lo stato RESERVED", taskName(0), 0);
                                    NODE_STATUS = RESERVED;
                                    status = forwardMsg(&in_msg, current_route->rasp_id_prev, NULL, true);
                                }
                            }
                            else{
                                status = handleMsg(&in_msg, false, RESERVED, RESERVED, "TRAIN_OK");
                            }
                        }
                        else if (strcmp(in_msg.command, "NOT_OK") == 0){
                            reset_status = resetNodeStatus();
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato NOT_RESERVED", taskName(0), 0);
                            logMessage("[t4] inoltro il messaggio al next o prev", taskName(0), 0);
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
                        if (strcmp(in_msg.command, "NOT_OK") == 0){
                            reset_status = resetNodeStatus();
                            logMessage("[t0] Preselection", taskName(0), 0);
                            logMessage("[t1] setto lo stato", taskName(0), 0);
                            logMessage("[t58] Resetto a NOT_RESERVED", taskName(0), 0);
                            status = forwardNotOk(&in_msg, in_msg.sender_id);
                        }
                        else{
                            status = handleErrorMsg(&in_msg, "RESERVED");
                        }
                        break;
                    default:
                        memset(log_msg, 0, 100);
                        snprintf(log_msg, 100, "Stato del nodo non riconosciuto : %i", NODE_STATUS);
                        logMessage(log_msg, taskName(0), 1);
                        break;
                    }
                    if(status != E_SUCCESS){
                        logMessage(errorDescription(status), taskName(0), 2);
                    }
                    if(reset_status != E_SUCCESS){
                        logMessage(errorDescription(reset_status), taskName(0), 2);
                    }
                }
            }
            // resetto il timer per la diagnostica (ho ricevuto un messaggio)
            startDiagnTime = tickGet();
        }
    }
}

void controlDestructor(int sig){
    if(resetNodeStatus() != E_SUCCESS){
        logMessage("Errore nella chiusura del control Task", taskName(0), 2);
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