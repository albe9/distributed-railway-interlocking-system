#include "diagnosticsTask.h"

void diagnosticsMain(){

    bool in_ping_fail_safe = false;

    //aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, diagnosticsDestructor);

    logMessage("-----Inizio diagnostica", taskName(0), 1);

    // Segnaliamo lo start della diagniostica al wifi task
    logMessage("-----[t22] Acquisisco il semaforo di status", taskName(0), 0);
    if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
    logMessage("-----[t25] Setto la flag di status", taskName(0), 0);
    ping_status = STARTING;
    logMessage("-----[t24] Rilascio il semaforo di status", taskName(0), 0);
    if(semGive(WIFI_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);

    // Mettiamoci in attesa del timeout per le risposte ai ping
    logMessage("-----[t33] Attesa risposta al ping", taskName(0), 0);
    taskDelay(TICKS_TO_SECOND * 2);

    // Accesso alle variabili globali tramite semaforo
    if(semTake(WIFI_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
    logMessage("-----[t34] Acquisisco il semaforo di status", taskName(0), 0);
    // Fermiamo la procedura di ping
    ping_status = ENDING;
    // in base al successo o meno dei ping impostiamo la variabile globale di ping fail safe
    logMessage("-----[t35] Controllo esito della diagnostica", taskName(0), 0);
    if(ping_success){
        in_ping_fail_safe = FALSE;
    }
    else{
        in_ping_fail_safe = TRUE;
        logMessage("-----Fallimento del task di ping", taskName(0), 1);
        // Notichiamo l'host
        //
        //
        // TODO: notificare l'host
        //
        //
        logMessage("-----[da implementare] Host notificato", taskName(0), 1);
    }
    // resettiamo la flag 
    ping_success = false;
    logMessage("-----[t36] Rilascio il semaforo", taskName(0), 0);
    if(semGive(WIFI_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);

    // Logghiamo la fine della diagnostica
    char log_msg[100];
	snprintf(log_msg, 100, "-----Diagnostica finita: in_ping_fail_safe = %d", in_ping_fail_safe);
    logMessage(log_msg, taskName(0), 1);

    // Scriviamo l'esito nell'area di memoria condivisa con il task di controllo
    if(semTake(CONTROL_DIAG_SEM, WAIT_FOREVER) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);
    logMessage("-----[t55] Acquisisco il semaforo", taskName(0), 0);
    logMessage("-----[t20] Scrivo esito diagnostica", taskName(0), 0);
    diag_ended = true;
    if(in_ping_fail_safe){
        diag_success = false;
    }
    else{
        diag_success = true;
    }
    logMessage("-----[t21] Rilascio il semaforo", taskName(0), 0);
    if(semGive(CONTROL_DIAG_SEM) < 0) logMessage(errorDescription(E_DEFAUL_ERROR), taskName(0), 2);

}

void diagnosticsDestructor(int sig){
    // Aggiungere elementi da rimuovere prima di un reload
    taskDelete(0);
}