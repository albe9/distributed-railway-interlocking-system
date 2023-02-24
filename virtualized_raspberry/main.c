#include "global_variables.h"
#include "wifiTask.h"
#include "initTask.h"
#include "my_debug.h"

int main(int argc, char *argv[]){

    if(argc > 1){
        
        
        strcpy(RASP_IP,argv[1]);    
        RASP_ID = atoi(argv[2]);
        
        // printf("ip : %s id : %i\n",RASP_IP,RASP_ID);
    }
    
    
    startDebug();
    initMain();
    

    return(0);
    
}

