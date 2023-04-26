#ifndef INCLUDES_POSITIONINGTASK_H_
#define INCLUDES_POSITIONINGTASK_H_

#include "global_variables.h"
#include "logTask.h"

#define MAX_POSITIONING 100
#define MIN_POSITIONING 0

#define PROB_ERROR 90 //%

extern void positioningMain();
extern exit_number positioning();
extern bool error_generator();



#endif /* INCLUDES_POSITIONINGTASK_H_ */
