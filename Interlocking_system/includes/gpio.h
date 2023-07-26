/*
* API per l'uso dei pin hardware
*   LED , BUTTON
*/

#ifndef GPIO_H
#define GPIO_H

#include "vxWorks.h"
#include "taskLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "sysLib.h"
#include <stdbool.h>
#include <subsys/gpio/vxbGpioLib.h>

#define OUT 1
#define IN 0

#define HIGH 1
#define LOW 0

#define LED_RED 23
#define LED_GREEN 24
#define LED_BLU 22
#define BUTTON 27

void turnOnLed(UINT32 pinNumber);

void turnOffLed(UINT32 pinNumber);

bool button_is_pressed(UINT32 pinNumber);

#endif /* GPI_H */