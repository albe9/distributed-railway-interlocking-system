#include <stdio.h>
#include <string.h>

/*
    test string: 
    Time:1673204636,Route:a,rasp_id_prev:0,rasp_id_next:2,prev_ip:127.0.0.1,next_ip:127.0.0.1,Stop:1

*/

int main(){

    char test_msg[] = "Time:1673204636,Route:a,rasp_id_prev:0,rasp_id_next:2,prev_ip:127.0.0.1,next_ip:127.0.0.1,Stop:1"; 
    char *token;
    char *key;
    token = strtok(test_msg, ",");

    while(token != NULL){
        printf("%s\n",token);
        token = strtok(NULL, ",");
        
        key = strtok(token, ":");
        while(key != NULL){
            printf("%s\n",key);
            key = strtok(NULL, ",");
            
        }
    }

    return(0);
    
}