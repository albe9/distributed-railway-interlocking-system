#include "destructorTask.h"


void destructorMain(void){

    // Invio un signal a tutti i task in modo tale che possano deallocare tutte le risorse prima di terminare
    if(kill(WIFI_TID, SIGUSR1) < 0){
        perror("\nErrore nell'invio del messaggio di kill al wifiTask");
    }
    if(kill(CONTROL_TID, SIGUSR1) < 0){
        perror("\nErrore nell'invio del messaggio di kill al ctrlTask");
    }
    if(kill(DIAGNOSTICS_TID, SIGUSR1) < 0){
        perror("\nErrore nell'invio del messaggio di kill al diagTask");
    }
    if(kill(TIMER_TID, SIGUSR1) < 0){
        perror("\nErrore nell'invio del messaggio di kill al timerTask");
    }
    if(kill(LOG_TID, SIGUSR1) < 0){
        perror("\nErrore nell'invio del messaggio di kill al logTask");
    }

    printf("\nChiusura destructorTask");

}