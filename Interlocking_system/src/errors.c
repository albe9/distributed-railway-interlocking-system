#include "errors.h"

static char defaultErrorMsg[50];

char *errorDescription(exit_number exit_number) {


    switch(exit_number) {
        //LOG
        case E_LOG_EMPTY: return "LOG FILE IS 0 SIZE"; break;
        case E_RESUME_LOG: return "UNABLE TO RESUME LOG TASK"; break;
        case E_LOG_OPEN: return "UNABLE TO OPEN LOG FILE"; break;
        
        //SOCKET
        case E_CONN_REFUSED: return "UNABLE TO CONNECT TO HOST, CONNECTION REFUSED"; break;
        case E_DEFAUL_ERROR: return strerror(errno); break;
        case E_SUCCESS: return "OK"; break;
        case E_INVALID_ID: return "INVALID CONNECTION SERVER ID"; break;
        case E_MAX_CONNECTION_NUMBER: return "REACHED THE MAXIMUM NUMBER OF CONNECTIONS"; break;
        case E_DISCONNECTION: return "NODE DISCONNECTION"; break;
        case E_NODE_NOTFOUND: return "CONNECTION TO NODE NOT FOUND"; break;
        
        //PING
        case E_PING_ACK: return "RECEIVED MORE PING_ACK THEN NUMBER OF ACTIVE CONNECTIONS"; break;

        //MSG
        case E_CLOSE: return "CLOSE MESSAGE BY HOST"; break;
        //TIME
        case E_SET_TIME: return "UNABLE TO SET TIME"; break;

        //PARSING
        case E_PARSING: return "PARSING ERROR"; break;

        //POSITIONING
        case E_POSITIONING: return "POSITIONING ERROR"; break;

        default: 
            snprintf(defaultErrorMsg, 50, "UNKNOW ERROR : %i", exit_number);
            return (defaultErrorMsg); 
            break;
    }
}