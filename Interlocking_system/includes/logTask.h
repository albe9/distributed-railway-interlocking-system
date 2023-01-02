/*
 * logTask.h
 *
 *  Created on: Dec 30, 2022
 *      Author: alber
 */

#ifndef INCLUDES_LOGTASK_H_
#define INCLUDES_LOGTASK_H_



#include <fcntl.h>
#include <semLib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <taskLib.h>
#include <time.h>
#include <unistd.h>


#include "global_variables.h" 

SEM_ID SEM_LOG;

extern void logMessage(char* msg, char* task_name);
extern void logInit(void);
extern void mytest(void);
extern void setCurrentTime(void);

#endif /* INCLUDES_LOGTASK_H_ */
