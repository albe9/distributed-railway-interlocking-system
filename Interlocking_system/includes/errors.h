#ifndef ERRORS_H
#define ERRORS_H

#include <errno.h>
#include <string.h>


typedef enum {
    //LOG
    E_LOG_EMPTY = -5,
    E_LOG = -4,
    E_LOG_OPEN = -3,
    
    //SOCKET
    E_CONN_REFUSED = -2,
    E_DEFAUL_ERROR = -1,
    E_SUCCESS = 0,
    E_INVALID_ID,
    E_MAX_CONNECTION_NUMBER,
    E_DISCONNECTION,
    E_NODE_NOTFOUND,
    
    //PING
    E_PING,
    E_PING_REQ,
    E_PING_ACK,
    E_PING_ERROR,

    //MSG
    E_CLOSE,

    //TIME
    E_SET_TIME,

    //PARSING
    E_PARSING,
    E_PARSING_DATA_EMPTY,
    E_PARSING_EMPTY,


    //POSITIONING
    E_POSITIONING
    
}exit_number;
 
extern char *errorDescription(exit_number exit_number);

#endif
