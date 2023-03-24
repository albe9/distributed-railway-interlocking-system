#include "controlTask.h"



extern void forwardMsg(tpcp_msg* msg, int receiver_id, char* command){


    if(command != NULL){
        memset(msg->command , 0, sizeof(msg->command));
        strcpy(msg->command, command);
    }

    msg->recevier_id = receiver_id;
    msg->sender_id = RASP_ID;
    msgQSend(OUT_CONTROL_QUEUE, (char*)msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
    
}

void controlMain(void){

    tpcp_msg in_msg;
    tpcp_status NODE_STATUS=NOT_RESERVED;
    int HOST_ID = 0;    //per adesso gestiamo un solo host

    while(true){
        ssize_t byte_recevied = msgQReceive(IN_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER);

        char msg[100];
        snprintf(msg, 100, "command :%s sender :%i recivier:%i route:%i", in_msg.command, in_msg.sender_id, in_msg.recevier_id, in_msg.route_id);
        logMessage(msg, taskName(0));


        // forwardMsg(&in_msg, 2, "TEST");

        
        route* current_route = NULL;
        for(int route_idx=0; route_idx<route_count; route_idx++){
            if(node_routes[route_idx].route_id == in_msg.route_id){
                current_route = &node_routes[route_idx];
                break;
            }
        }

        if( current_route == NULL){
            // TODO gestire risposta quando la route non è presente
        }
        else{

            //Gestisco i messaggi ricevuti dal task wifi

            
                // campo command:

                // -REQ
                // -WAIT_ACK
                // -ACK
                // -NACK

                // -WAIT_COMMIT
                // -COMMIT
                // -NOT_COMMIT

                // -WAIT_AGREE
                // -AGREE
                // -DISAGREE

            
            //TODO gestire tutti i msg ricevuti in uno stato che non li supporta
            if (strcmp(in_msg.command, "REQ") == 0){
                if(NODE_STATUS != NOT_RESERVED){
                    //TODO gestire
                }
                else{
                    if(current_route->rasp_id_next != TAIL_ID){
                        NODE_STATUS = WAIT_ACK;
                        forwardMsg(&in_msg, current_route->rasp_id_next, NULL);
                        // in_msg.recevier_id = current_route->rasp_id_next;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                    else{
                        NODE_STATUS = WAIT_COMMIT;
                        forwardMsg(&in_msg, current_route->rasp_id_prev, "ACK");
                        // memset(in_msg.command , 0, sizeof(in_msg.command));
                        // strcpy(in_msg.command, "ACK");
                        // in_msg.recevier_id = current_route->rasp_id_prev;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                }
            }
            else if (strcmp(in_msg.command, "ACK") == 0){
                if(NODE_STATUS != WAIT_ACK){
                    //TODO gestire 
                }
                else{
                    if(current_route->rasp_id_prev != HOST_ID){
                        NODE_STATUS = WAIT_COMMIT;
                        forwardMsg(&in_msg, current_route->rasp_id_prev, NULL);
                        // in_msg.recevier_id = current_route->rasp_id_prev;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                    else{
                        NODE_STATUS = WAIT_AGREE;
                        forwardMsg(&in_msg, current_route->rasp_id_next, "COMMIT");
                        // memset(in_msg.command , 0, sizeof(in_msg.command));
                        // strcpy(in_msg.command, "COMMIT");
                        // in_msg.recevier_id = current_route->rasp_id_next;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                }
            }
            else if (strcmp(in_msg.command, "COMMIT") == 0){
                if(NODE_STATUS != WAIT_COMMIT){
                    //TODO gestire
                }
                else{
                    if(current_route->rasp_id_next != TAIL_ID){
                        NODE_STATUS = WAIT_AGREE;
                        forwardMsg(&in_msg, current_route->rasp_id_next, NULL);
                        // in_msg.recevier_id = current_route->rasp_id_next;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                    else{
                        NODE_STATUS = RESERVED;
                        forwardMsg(&in_msg, current_route->rasp_id_prev, "AGREE");
                        // memset(in_msg.command , 0, sizeof(in_msg.command));
                        // strcpy(in_msg.command, "AGREE");
                        // in_msg.recevier_id = current_route->rasp_id_prev;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                }
            }
            else if (strcmp(in_msg.command, "ACK") == 0){
                if(NODE_STATUS != WAIT_ACK){
                    //TODO gestire 
                }
                else{
                    if(current_route->rasp_id_prev != HOST_ID){
                        NODE_STATUS = WAIT_COMMIT;
                        forwardMsg(&in_msg, current_route->rasp_id_prev, NULL);
                        // in_msg.recevier_id = current_route->rasp_id_prev;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                    else{
                        NODE_STATUS = WAIT_AGREE;
                        forwardMsg(&in_msg, current_route->rasp_id_next, );
                        // memset(in_msg.command , 0, sizeof(in_msg.command));
                        // strcpy(in_msg.command, "COMMIT");
                        // in_msg.recevier_id = current_route->rasp_id_next;
                        // msgQSend(OUT_CONTROL_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL);
                    }
                }
            }
        }
    }
}


