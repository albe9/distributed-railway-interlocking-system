/*
 * logTask.h
 *
 *  Created on: Dec 30, 2022
 *      Author: alber
 */

#ifndef INCLUDES_LOGTASK_H_
#define INCLUDES_LOGTASK_H_

#include "stdio.h"
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>




extern void logMessage(char* msg, char* task_name);
extern void logInit(void);
void mytest(void);


#endif /* INCLUDES_LOGTASK_H_ */
