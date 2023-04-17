#include "positioningTask.h"
#include "global_variables.h"

bool ERROR_ACTIVATED = false;

int positioning(){
    int positioning = MIN_POSITIONING;
    while (positioning < MAX_POSITIONING)
    {
        error_generator();
        if (ERROR_ACTIVATED == true){
            return E_POSITIONING;
        }
        positioning = positioning + 1;
    }
    
    sleep(3);

    return positioning;
}

int error_generator(){
    srand(time(0));
    int generate_error = rand() % PROB_ERROR + 1;
    if (generate_error == 1) {
        ERROR_ACTIVATED = true;
    }
}