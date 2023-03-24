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

#endif /* INCLUDES_CONTROLTASK_H_ */