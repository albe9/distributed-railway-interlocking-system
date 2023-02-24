#ifndef ERRORS_H
#define ERRORS_H

#include <errno.h>
#include <string.h>

enum _EXIT_NUMBER {
    //SOCKET
    E_CONN_REFUSED = -2,
    E_DEFAUL_ERROR = -1,
    E_SUCCESS = 0,
    E_INVALID_ID,
    E_MAX_CONNECTION_NUMBER,

    //TIME
    E_SET_TIME,

    //PARSING
    E_PARSING,

    //LOG
    E_LOG
};

typedef enum _EXIT_NUMBER exit_number;

const char *errorDescription(exit_number exit_number) {
    switch(exit_number) {
        case E_DEFAUL_ERROR: return strerror(errno); break;
        case E_SUCCESS: return "OK"; break;
        case E_INVALID_ID: return "INVALID CONNECTION SERVER ID"; break;
        case E_MAX_CONNECTION_NUMBER: return "REACHED THE MAXIMUM NUMBER OF CONNECTIONS"; break;
        case E_CONN_REFUSED: return "UNABLE TO CONNECT TO HOST, CONNECTION REFUSED"; break;

        case E_SET_TIME: return "UNABLE TO SET TIME"; break;

        case E_PARSING: return "PARSING ERROR"; break;

        case E_LOG: return "UNABLE TO LOG"; break;

        default: return "UNKNOW ERROR"; break;
    }
}

#endif
