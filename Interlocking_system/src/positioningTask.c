#include "positioningTask.h"

void positioningMain(){
    exit_number status;
    if((status = positioning()) != E_SUCCESS){
        IN_POSITION = false;
        logMessage(errorDescription(status), taskName(0));
    }
    else{
        IN_POSITION = true;
    }
    taskResume(CONTROL_TID);
}

exit_number positioning(){

    if (error_generator() == true){
        return E_POSITIONING;
    }
    else{
        int positioning = MIN_POSITIONING;
        while (positioning < MAX_POSITIONING)
        {
            positioning = positioning + 1;
        }
        sleep(3);
        return E_SUCCESS;
    }
}

bool error_generator(){

    srand(time(0));
    int generate_error = rand() % 100;
    if (generate_error <= PROB_ERROR) {
        return true;
    }
    else{
        return false;
    }
}