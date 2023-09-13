#ifndef INCLUDES_TIMERTASK_H_
#define INCLUDES_TIMERTASK_H_

#include "global_variables.h"

extern void timerMain(void);
extern u_int64_t getTimeMicro(void);
extern void timerDestructor(int sig);


#endif /* INCLUDES_TIMERTASK_H_ */