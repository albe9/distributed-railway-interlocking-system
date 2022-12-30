/*
 * logTask.h
 *
 *  Created on: Dec 30, 2022
 *      Author: alber
 */

#ifndef INCLUDES_LOGTASK_H_
#define INCLUDES_LOGTASK_H_

#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


extern int LOG_FD;

extern void logMessage(char* msg);
extern void logInit(void);


#endif /* INCLUDES_LOGTASK_H_ */
