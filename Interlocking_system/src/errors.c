#include "errors.h"

char *errorDescription(exit_number exit_number) {
    switch(exit_number) {
        case E_DEFAUL_ERROR: return strerror(errno); break;
        case E_SUCCESS: return "OK"; break;
        case E_INVALID_ID: return "INVALID CONNECTION SERVER ID"; break;
        case E_MAX_CONNECTION_NUMBER: return "REACHED THE MAXIMUM NUMBER OF CONNECTIONS"; break;
        case E_CONN_REFUSED: return "UNABLE TO CONNECT TO HOST, CONNECTION REFUSED"; break;

        case E_SET_TIME: return "UNABLE TO SET TIME"; break;

        case E_PARSING: return "PARSING ERROR"; break;

        case E_LOG: return "UNABLE TO LOG"; break;

        case E_NODE_NOTFOUND: return "CONNECTION TO NODE NOT FOUND"; break;

        case E_POSITIONING: return "POSITIONING ERROR"; break;

        default: return "UNKNOW ERROR"; break;
    }
}