#include "positioningTask.h"

void positioningMain(){

    logMessage("[t83] Inizio posiTask", taskName(0), 0);
    exit_number status;
    if((status = positioning()) != E_SUCCESS){
        railswitch.in_position = false;
        logMessage(errorDescription(status), taskName(0), 2);
    }
    else{
        railswitch.in_position = true;
    }
    logMessage("[t80] Terminazione del task di positioning", taskName(0), 0);
    char msg[100];
    snprintf(msg, 100, "Deviatoio per rotta: %i   e in_position: %d", railswitch.last_route_id, railswitch.in_position);
    logMessage(msg, taskName(0), 1);
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
        logMessage("[t14] positioning avvenuto", taskName(0), 0);
        return E_SUCCESS;
    }
}

bool error_generator(){

    srand((int)time(0));
    int generate_error = rand() % 100;
    if (generate_error < PROB_ERROR) {
        return true;
    }
    else{
        return false;
    }
}