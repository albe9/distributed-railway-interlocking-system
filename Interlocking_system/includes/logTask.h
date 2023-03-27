/*
 * logTask.h
 *
 *  Created on: Dec 30, 2022
 *      Author: alber
 */

#ifndef INCLUDES_LOGTASK_H_
#define INCLUDES_LOGTASK_H_


#include "global_variables.h" 
#include <clockLib.h>
#include "wifiTask.h"


extern void logMessage(char* msg, char* task_name);
extern void logInit(void);
extern void mytest(void);

#endif /* INCLUDES_LOGTASK_H_ */
