#ifndef INCLUDES_CONTROLTASK_H_
#define INCLUDES_CONTROLTASK_H_

#include "global_variables.h"
#include "logTask.h"
#include "positioningTask.h"

typedef enum{
    NOT_RESERVED,
    WAIT_ACK,
    WAIT_COMMIT,
    WAIT_AGREE,
    POSITIONING,
    MALFUNCTION,
    RESERVED,
    TRAIN_IN_TRANSITION,
    FAIL_SAFE,
    PING_FAIL_SAFE

}tpcp_status;

extern void controlMain(void);
extern exit_number forwardNotOk(tpcp_msg* msg, int sender_id);
extern exit_number forwardMsg(tpcp_msg* msg, int receiver_id, char* command);       //inoltra il msg al nodo indicato, se command=NULL il comando non viene modificato
extern exit_number handleMsg(tpcp_msg* msg, bool direction, tpcp_status new_middleStatus, tpcp_status new_edgeStatus, char* new_command);
extern exit_number handleErrorMsg(tpcp_msg* msg, char* current_status);
extern void hookControlDelete(_Vx_TASK_ID tid);
extern exit_number resetNodeStatus();

#endif /* INCLUDES_CONTROLTASK_H_ */