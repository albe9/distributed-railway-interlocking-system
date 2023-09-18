#include "timerTask.h"

void timerMain(void){

	//aggiungo l'handler per il signal SIGUSR1
	signal(SIGUSR1, timerDestructor);

    TIMER_SEM = semBCreate(SEM_Q_FIFO, SEM_FULL);
	uint32_t currentTimerRegValue;
	bool flagOverflow = false;
	int overflowCounter = 0;
	while(true){
		currentTimerRegValue = sysTimestamp();
		if(currentTimerRegValue >= 4294967000 && !flagOverflow){
			flagOverflow = true;
		}
		else if(currentTimerRegValue <= 1000 && flagOverflow){
			overflowCounter ++;
	
            semTake(TIMER_SEM, WAIT_FOREVER);
			totalCurrentTimeMicro = (uint64_t)(overflowCounter * TIME_TO_GET_OVERFLOW_MICRO + currentTimerRegValue / TIMER_FREQ_MHz);
            semGive(TIMER_SEM);
			
			flagOverflow = false;
			// taskdelay di 76 secondi per evitare il busy sleep
			taskDelay(4560);
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
	time_reg_micro /= TIMER_FREQ_MHz;
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