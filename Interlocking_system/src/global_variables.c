#include "global_variables.h"

int RASP_ID = -1;
char HOST_IP[20];
char RASP_IP[20];
int SERVER_PORT = 6543;
int LOG_PORT = 6544;
int CURRENT_HOST = -1;
route *node_routes;
int route_count = 0;
int NODE_TYPE = -1;
bool IN_POSITION = false;
int log_status = LOG_ACTIVE;
bool ping_success = false;     // Risultato di una procedura di ping, può essere FAIL (0) o SUCCESS (1)
bool in_ping_fail_safe = FALSE;      // Indica se il nodo stesso è in fail safe (TRUE) oppure no (FALSE)
int ping_answers = 0;           // Numero di ack ricevuto in risposta ad una richiesta di ping