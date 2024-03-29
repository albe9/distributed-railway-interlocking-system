#include "global_variables.h"

int RASP_ID = -1;
char HOST_IP[20];
char RASP_IP[20];
bool SIM_SENSOR = false;
int SIM_SENSOR_OFF_TIME = 2;
int SERVER_PORT = 6543;
int LOG_PORT = 6544;
int LOG_FD = -1;
int CURRENT_HOST = -1;
route *node_routes;
int route_count = 0;
int NODE_TYPE = -1;
int log_status = LOG_ACTIVE;
bool ping_success = false;      // Risultato di una procedura di ping, può essere FAIL (0) o SUCCESS (1)
bool diag_success = false;      // Risultato diagnostica, variabile condivisa tra diag e control task
bool diag_ended = false;
int ping_answers = 0;           // Numero di ack ricevuto in risposta ad una richiesta di ping
bool sensor_on_detected = false;
railroad_switch railswitch = {.in_position = false, .last_route_id = 0};
uint64_t totalCurrentTimeMicro = 0;