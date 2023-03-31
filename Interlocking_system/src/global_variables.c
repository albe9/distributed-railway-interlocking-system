#include "global_variables.h"

int RASP_ID = -1;
char HOST_IP[20];
char RASP_IP[20];
int SERVER_PORT = 6543;
int LOG_PORT = 6544;
int LOG_PORT = 6544;
int CURRENT_HOST = -1;
route *node_routes;
int route_count = 0;
int NODE_TYPE = -1;
int log_status = LOG_ACTIVE;