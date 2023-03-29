import socket
import time
import sys

LOG_PORT = 6544  # Port to listen on (non-privileged ports are > 1023)
# HOST_IP_SUFFIX = "172.23.78."
HOST_IP_SUFFIX = "192.168.1.21"
HOST_ID = 0
HOST_IP = sys.argv[2]
TAIL_ID = -9999

def log_loop(n_nodes:int):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST_IP, LOG_PORT))       #accetta connessioni sull'indirizzo HOST_IP e porta PORT
        s.listen()
        # Creiamo una lista e aggiungiamo tutti i nodi che devono loggare
        log_connected_nodes = []
        for node_index in range(n_nodes):           
            log_conn, log_addr = s.accept()
            log_connected_nodes.append(log_conn)
        while True:    
            for node_index in range(n_nodes):
                # ricevi e printa tutto il messaggio di un nodo
                while True:                     
                        log_data = log_conn.recv(1024).decode()
                        print(log_data)
                        if not log_data:
                            print("\n")
                            break


log_loop(int(sys.argv[1]))