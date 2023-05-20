#include "diagnosticsTask.h"
void diagnosticsMain(){
    // TODO: logica di esecuzione
    logMessage("...Eseguito diagnostica...", taskName(0));
    taskResume(CONTROL_TID);
}
