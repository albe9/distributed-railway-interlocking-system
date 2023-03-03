#include "my_debug.h"

FILE *debug_file = NULL;

void startDebug(){
    
    if(RASP_ID > 0){
        char debug_path[50] = {0};
        snprintf(debug_path, 50, "./debug/debug_%i.txt", RASP_ID);
        debug_file = fopen(debug_path, "w");
    }
}

