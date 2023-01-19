import socket
import time
import sys

PORT = 6543  # Port to listen on (non-privileged ports are > 1023)
HOST_IP_SUFFIX = "172.23.78."


class Node:
    def __init__(self, rasp_id_prev: int, prev_ip: str, rasp_id_next: int, next_ip: str):
        self.rasp_id_prev = rasp_id_prev
        self.prev_ip = prev_ip
        self.rasp_id_next = rasp_id_next
        self.next_ip = next_ip
class Route:
    def __init__(self, route_id: int):
        self.route_id = route_id
        self.node_list = []
    def add_node(self, node_to_add: Node) -> int:
        self.node_list.append(node_to_add)
        return 1
    def remove_node(self, note_to_remove: Node) -> int:
        self.node_list.remove(note_to_remove)
        return 1


# I route sono dizionari con chiave id rotta e con valore un dizionario con chiave id del nodo e come valore informazioni del nodo nella rotta
# rasp_id 0 è stato assegnato all'host che ha ip 172.23.78.0
# rasp_id -9999 è stato assegnato al nodo [che non esiste] successivo al capolinea ed esso avrà ip -9999
# Gli ip sono stati assegnati a partire da quello dell'host, ad esempio il rasp 1 ha ip [172.23.78].1, il rasp 2 ha [172.23.78].2
# Per visualizzare queste rotte accedere a ProgettoSWE4ES>Diagrammi>Routes
Routes_test =  \
{
    #id univoco della rotta
    1: 
    {
        # id univoco del nodo in tutta la rete
        1 : 
        {
            "rasp_id_prev" : "0",
            "prev_ip" : HOST_IP_SUFFIX + "0",
            "rasp_id_next" : "2",
            "next_ip" : HOST_IP_SUFFIX + "2",            
        },

        2 :
        {
            "rasp_id_prev" : "1",
            "prev_ip" : HOST_IP_SUFFIX + "1",
            "rasp_id_next" : "3",
            "next_ip" : HOST_IP_SUFFIX + "3",    
       },

       3 :
       {
            "rasp_id_prev" : "2",
            "prev_ip" : HOST_IP_SUFFIX + "2",
            "rasp_id_next" : "4",
            "next_ip" : HOST_IP_SUFFIX + "4",  
       },

       4 :
       {
            "rasp_id_prev" : "3",
            "prev_ip" : HOST_IP_SUFFIX + "3",
            "rasp_id_next" : "5",
            "next_ip" : HOST_IP_SUFFIX + "5",  
       },

       5 :
       {
            "rasp_id_prev" : "4",
            "prev_ip" : HOST_IP_SUFFIX + "4",
            "rasp_id_next" : "-9999",
            "next_ip" : HOST_IP_SUFFIX + "3",  
       },
    },

    2: 
    {
        1 :
        {
            
        },

        2 :
        {
            
        },

        3 :
        {
            
        },
    },
}

Routes_real = {
    1:
    {
        1 :
        {
            "rasp_id_prev" : "0",
            "rasp_id_next" : "2",
            "prev_ip" : "172.23.78.253",
            "next_ip" : "172.23.78.253",
        },
        2 :
        {
            "rasp_id_prev" : "1",
            "rasp_id_next" : "-1",
            "prev_ip" : "172.23.78.253",
            "next_ip" : "-1",
        },
    },
}

def query_node_data(rasp_id, routes):

    """
        Estrae le informazioni relative ad un singolo nodo dalla mappa dei tracciati
    """

    node_data = \
    {
        "prev_node_ips" : set(),
        "next_node_ips" : set(),
        "routes" : []
        
    }


    for route_id, route_data in routes.items():
        if rasp_id in route_data:
            node_data["prev_node_ips"].add(route_data[rasp_id]["prev_ip"])
            node_data["next_node_ips"].add(route_data[rasp_id]["next_ip"])
            node_data["routes"].append([route_id,route_data[rasp_id]["rasp_id_prev"],route_data[rasp_id]["rasp_id_next"]]) 

    return node_data

def make_config_string(rasp_id, routes):
    """
        Definizione protocollo : 
            -primo pacchetto:
                -Time since epoch locale,
                -Numero di nodi precedenti,
                -Numero di nodi successivi,
                -Numero di route di cui fa parte il nodo,
            -secondo pacchetto:
                -prev_node_ips,
            -terzo pacchetto:
                -next_node_ips,
            -quarto pacchetto e successivi:
                -route_id,
                -rasp_id_prev,
                -rasp_id_next,

        Separatori : 
        ';' tra due tipologie di pacchetto differenti
        '-' tra due pacchetti dello stesso tipo (accade solo con i pacchetti di tipo 4)
        ',' tra i dati dello stesso pacchetto        
        
    """

    config_string = ""
    #primo pacchetto 
    node_data = query_node_data(rasp_id, routes)
    config_string += f"{int(time.time())},"
    config_string += f"{len(node_data['prev_node_ips'])},"
    config_string += f"{len(node_data['next_node_ips'])},"
    config_string += f"{len(node_data['routes'])};"

    #secondo pacchetto
    for ip in node_data['prev_node_ips']:
        config_string += f"{ip},"
    config_string = config_string.removesuffix(',')
    config_string += ";" 
    #terzo pacchetto
    for ip in node_data['next_node_ips']:
        config_string += f"{ip},"
    config_string = config_string.removesuffix(',')
    config_string += ";" 
    #quarto pacchetto e successivi
    for route_id, prev_id, next_id in node_data['routes']:
        config_string += f"{route_id},{prev_id},{next_id}-"
    config_string = config_string.removesuffix('-')
    config_string += ";"

    return config_string




def server_loop(node_num, routes):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('0.0.0.0', PORT))       #accetta connessioni da qualsiasi indirizzo sulla porta PORT
        s.listen()
        for node_idx in range(node_num):
            conn, addr = s.accept()
            with conn:
                print(f"Connected by {addr}")
                rasp_msg = conn.recv(1024)
                print(f"messaggio ricevuto : {rasp_msg.decode()}")

                rasp_msg = rasp_msg.decode().strip()
                rasp_id = int(rasp_msg.split("RASP_ID : ")[1])

                msg = make_config_string(rasp_id, routes)
                conn.send(msg.encode())
            
        s.close()


def main():
    # server_loop(int(sys.argv[1]), Routes_real)
    print(query_node_data(1, Routes_real))

if __name__ == "__main__":
    main()

