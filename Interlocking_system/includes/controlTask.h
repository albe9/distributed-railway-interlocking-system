#ifndef INCLUDES_CONTROLTASK_H_
#define INCLUDES_CONTROLTASK_H_

#include "global_variables.h"
#include "logTask.h"

typedef enum{
    NOT_RESERVED,
    WAIT_ACK,
    WAIT_COMMIT,
    WAIT_AGREE,
    POSITIONING,
    MALFUNCTION,
    RESERVED,
    TRAIN_IN_TRANSITION

}tpcp_status;


extern void controlMain(void);
extern void forwardMsg(tpcp_msg* msg, int receiver_id, char* command);       //inoltra il msg al nodo indicato, se command=NULL il comando non viene modificato
extern void handleMsg(tpcp_msg* msg, bool direction, tpcp_status new_middleStatus, tpcp_status new_edgeStatus, char* new_command);
extern void hookControlDelete(_Vx_TASK_ID tid);
extern void resetNodeStatus();

#endif /* INCLUDES_CONTROLTASK_H_ */