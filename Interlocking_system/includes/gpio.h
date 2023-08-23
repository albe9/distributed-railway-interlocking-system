/*
* API per l'uso dei pin hardware
*   LED , BUTTON
*/

#ifndef GPIO_H
#define GPIO_H

#include "global_variables.h"
#include <subsys/gpio/vxbGpioLib.h>

void changeLedColor(status_color node_status_col);

void setLedColor(color new_color);

bool readButton();

#endif /* GPI_H */