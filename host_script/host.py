import socket
import time
import sys


HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 6543  # Port to listen on (non-privileged ports are > 1023)


 # Da scegliere la sintassi, per adesso dict che associa ad ogni nodo(Tramite RASP_ID) il nodo precedente e successivo
Routes_test =  \
{
    1:
    {
        1 :
        {
            "rasp_id_prev" : "0",
            "rasp_id_next" : "2",
            "prev_ip" : "127.0.0.1",
            "next_ip" : "127.0.0.1",
        },
        2 :
        {
            "rasp_id_prev" : "1",
            "rasp_id_next" : "-1",
            "prev_ip" : "127.0.0.1",
            "next_ip" : "-1",
        },
    },
    2: 
    {
        1 :
        {
            "rasp_id_prev" : "0",
            "rasp_id_next" : "3",
            "prev_ip" : "127.0.0.1",
            "next_ip" : "192.168.1.1",
        },
    },
    3: 
    {
        2 :
        {
            "rasp_id_prev" : "0",
            "rasp_id_next" : "2",
            "prev_ip" : "127.0.0.1",
            "next_ip" : "127.0.0.1",
        },
    }
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
    server_loop(int(sys.argv[1]), Routes_real)

if __name__ == "__main__":
    main()

