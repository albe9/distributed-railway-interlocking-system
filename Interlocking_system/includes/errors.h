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
    E_DISCONNECTION,
    E_NODE_NOTFOUND,
    //MSG
    E_CLOSE,

    //TIME
    E_SET_TIME,

    //PARSING
    E_PARSING,

    //LOG
    E_LOG
};

typedef enum _EXIT_NUMBER exit_number;

extern char *errorDescription(exit_number exit_number);

#endif
