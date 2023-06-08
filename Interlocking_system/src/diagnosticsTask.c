#include "diagnosticsTask.h"

void diagnosticsMain(){
    //aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, diagnosticsDestructor);
    logMessage("-----Inizio diagnostica", taskName(0));

    // Creiamo il messaggio da passare al task WiFi per avviare la procedura di ping
    tpcp_msg ping_start_msg = {"PING_START", RASP_ID, RASP_ID, ROUTE_ID_PING, HOST_ID_PING};

    // Inviamo il messaggio di PING_START utilizzando la coda
    if(msgQSend(OUT_DIAGNOSTICS_QUEUE, (char*)&ping_start_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
        logMessage("-----Problema invio verso la coda", taskName(0));
    }

    // Mettiamoci in attesa di un'eventuale risposta del task WiFi alla procedura di ping sulla coda
    tpcp_msg in_msg;
    ssize_t byte_recevied = msgQReceive(IN_DIAGNOSTICS_QUEUE, (char*)&in_msg, sizeof(tpcp_msg), TICKS_TO_SECOND * 15);
    if(byte_recevied < 0){
        // Se il timeout è stato superato la procedura di ping è da considerarsi fallita
        if(strcmp(strerror(errno), "S_objLib_OBJ_TIMEOUT") == 0){
            // Scriviamo una variabile globale
            ping_result = FAIL;
            // Notifichiamo del fallimento il task WiFi
            tpcp_msg ping_fail_msg = {"PING_FAIL", RASP_ID, RASP_ID, ROUTE_ID_PING, HOST_ID_PING};
            if(msgQSend(OUT_DIAGNOSTICS_QUEUE, (char*)&ping_fail_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
                logMessage("-----Problema invio verso la coda", taskName(0));
            }
            logMessage("-----Fallimento del task di ping", taskName(0));
            // Notichiamo l'host
            //
            //
            // TODO: notificare l'host
            //
            //
            logMessage("-----[da implementare] Host notificato", taskName(0));              
        }
        // Non abbiamo ricevuto nessun messaggio da WiFi e abbiamo un errore diverso da timeout, da gestire 
        else{
            logMessage(strerror(errno), taskName(0));
        }
    }
    // Se riceviamo una risposta dal task WiFi
    else{
        // Controlliamo che la risposta sia stata "PING_SUCCESS"
        if(strcmp(in_msg.command, "PING_SUCCESS") != 0){
            logMessage("-----Qualcosa è andato storto, non si è ricevuto PING_SUCCESS", taskName(0));
        }
        else{
            // La diagnostica non ha incontrato problemi
            ping_result = SUCCESS;
            // Notifichiamo del successo il task WiFi
            tpcp_msg ping_finished_msg = {"PING_FINISHED", RASP_ID, RASP_ID, ROUTE_ID_PING, HOST_ID_PING};
            if(msgQSend(OUT_DIAGNOSTICS_QUEUE, (char*)&ping_finished_msg, sizeof(tpcp_msg), WAIT_FOREVER, MSG_PRI_NORMAL) != OK){
                logMessage("-----Problema invio verso la coda", taskName(0));
            }
        }

        
    }

    // In base a come è andata la procedura [SUCCESS (1) o FAIL (0)] di diagnostica eseguiamo diverse azioni
    if(ping_result == FAIL){
        // Se è la procedura ha terminanto con un FAIL impostiamo la variabile globale di ping fail safe, che viene monitorata dal ControlTask in attesa
        in_ping_fail_safe = TRUE;
    }
    else{
        in_ping_fail_safe = FALSE;
    }

    // Logghiamo la fine della diagnostica e facciamo il resume del task di controllo
    char log_msg[100];
	snprintf(log_msg, 100, "-----Diagnostica finita: risultato del ping %d  -  in_ping_fail_safe %d", ping_result, in_ping_fail_safe);
    logMessage(log_msg, taskName(0));
    taskResume(CONTROL_TID);
}

void diagnosticsDestructor(int sig){
    // Aggiungere elementi da rimuovere prima di un reload
    taskDelete(0);
}