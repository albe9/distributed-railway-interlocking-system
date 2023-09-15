#include "timerTask.h"

void timerMain(void){

	//aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, timerDestructor);

    TIMER_SEM = semBCreate(SEM_Q_FIFO, SEM_FULL);
	uint32_t currentTimeNano;
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
			totalCurrentTimeMicro = (uint64_t)(overflowCounter * 79536431 + currentTimeNano/54);
            semGive(TIMER_SEM);
			
			flagOverflow = false;
			// taskdelay di 70 secondi per evitare il busy sleep
			taskDelay(4200);
		}
	}
}

uint64_t getTimeMicro(void)
{
	uint64_t time_micro = 0;
	semTake(TIMER_SEM, WAIT_FOREVER);
	time_micro = totalCurrentTimeMicro;
	semGive(TIMER_SEM);
	
	// printf("Timer task ha riportato : %llu\n", time_micro);
	// sommo al tempo totale dato dagli overflow il tempo attuale in microsecondi
	uint32_t time_reg_micro = sysTimestamp();
	// printf("sysTimestamp            : %llu\n", time_reg_micro);
	time_reg_micro /= 54;
	// printf("sysTimestamp / 54       : %llu\n", time_reg_micro);
	
	// printf("%llu\n", (uint64_t)time_reg_micro);
	time_micro += (uint64_t)time_reg_micro;
	// printf("Total micro             : %llu\n", time_micro);

	return time_micro;
}

void timerDestructor(int sig){
	// Aggiungere elementi da rimuovere prima di un reload
    taskDelete(0);
}