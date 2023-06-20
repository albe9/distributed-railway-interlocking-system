#include "diagnosticsTask.h"

void diagnosticsMain(){
    //aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, diagnosticsDestructor);

    //TODO aggiungere check per verificare che il task di controllo sia sospeso, altrimenti si attende
    logMessage("-----Inizio diagnostica", taskName(0));

    // Segnaliamo lo start della diagniostica al wifi task
    if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
    ping_status = STARTING;
    if(semGive(WIFI_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));

    // Mettiamoci in attesa del timeout per le risposte ai ping
    taskDelay(TICKS_TO_SECOND * 15);

    // Accesso alle variabili globali tramite semaforo
    if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));
    // Fermiamo la procedura di ping
    ping_status = ENDING;
    // in base al successo o meno dei ping impostiamo la variabile globale di ping fail safe, che verrà controllata dal ControlTask dopo il resume
    if(ping_success){
        in_ping_fail_safe = FALSE;
    }
    else{
        in_ping_fail_safe = TRUE;
        logMessage("-----Fallimento del task di ping", taskName(0));
        // Notichiamo l'host
        //
        //
        // TODO: notificare l'host
        //
        //
        logMessage("-----[da implementare] Host notificato", taskName(0));
    }
    // resettiamo la flag 
    ping_success = false;
    if(semGive(WIFI_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0));

    // Logghiamo la fine della diagnostica e facciamo il resume del task di controllo
    char log_msg[100];
	snprintf(log_msg, 100, "-----Diagnostica finita: in_ping_fail_safe = %d", in_ping_fail_safe);
    logMessage(log_msg, taskName(0));
    taskResume(CONTROL_TID);
}

void diagnosticsDestructor(int sig){
    // Aggiungere elementi da rimuovere prima di un reload
    taskDelete(0);
}