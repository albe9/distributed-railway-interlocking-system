#include "gpio.h"

void turnOnLed(UINT32 pinNumber){
    vxbGpioSetDir(pinNumber, OUT);
    vxbGpioSetValue(pinNumber, HIGH);

}

void turnOffLed(UINT32 pinNumber){
    vxbGpioSetValue(pinNumber, LOW);
    vxbGpioFree(pinNumber);
}

bool button_is_pressed(UINT32 pinNumber){
    vxbGpioSetDir(pinNumber, IN);
    return (vxbGpioGetValue(pinNumber) == 1);
}