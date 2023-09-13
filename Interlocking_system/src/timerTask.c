#include "timerTask.h"

void timerMain(void){

	//aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, timerDestructor);

    TIMER_SEM = semBCreate(SEM_Q_FIFO, SEM_FULL);
	UINT32 currentTimeNano;
	bool flagOverflow = false;
	int overflowCounter = 0;
	while(true){
		currentTimeNano = sysTimestamp();
		if(currentTimeNano >= 4294967000 && !flagOverflow){
			flagOverflow = true;
		}
		else if(currentTimeNano <= 1000 && flagOverflow){
			overflowCounter ++;
			// printf("%i\n", overflowCounter);
            semTake(TIMER_SEM, WAIT_FOREVER);
            // Esprimiamo il tempo in microsecondi totali trascorsi  (2^32-1)/54 = numero di microsecondi che impiega sysTimestamp ad effettuare overflow
			totalCurrentTimeMicro = overflowCounter * 79536431 + currentTimeNano/54;
            semGive(TIMER_SEM);
			
			flagOverflow = false;
			// taskdelay di 70 secondi per evitare il busy sleep
			taskDelay(4200);
		}
	}
}

u_int64_t getTimeMicro(void)
{
	UINT64 time_micro = 0;
	semTake(TIMER_SEM, WAIT_FOREVER);
	time_micro = totalCurrentTimeMicro;
	semGive(TIMER_SEM);
	// sommo al tempo totale dato dagli overflow il tempo attuale in microsecondi
	time_micro += (sysTimestamp())/54;

	return time_micro;
}

void timerDestructor(int sig){
	// Aggiungere elementi da rimuovere prima di un reload
    taskDelete(0);
}