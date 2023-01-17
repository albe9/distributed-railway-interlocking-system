#include "global_variables.h"
#include "wifiTask.h"
#include "initTask.h"


int main(int argc, char *argv[]){

    if(argc > 1){
        int rasp_id = -1;
        char rasp_ip[20];
        
        strcpy(rasp_ip,argv[1]);    
        rasp_id = atoi(argv[2]);
        
        printf("ip : %s id : %i\n",rasp_ip,rasp_id);
    }
    

    //initMain();

    return(0);
    
}

