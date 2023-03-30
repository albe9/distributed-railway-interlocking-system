import socket
import time
import sys
import threading
import typing

LOG_PORT = 6544  # Port to listen on (non-privileged ports are > 1023)
# HOST_IP_SUFFIX = "172.23.78."
HOST_IP_SUFFIX = "192.168.1.21"
HOST_ID = 0
HOST_IP = sys.argv[2]
TAIL_ID = -9999

def receiveMessage(conn: socket):
    # ricevi e printa tutto il messaggio di un nodo
     while True:                     
            log_data = conn.recv(1024).decode()
            print(log_data)
            if not log_data:
                print("\n \n \n")
                break

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
        # Creiamo una lista di thread, uno per ogni socket
        log_threads = []
        for conn_index in range(len(log_connected_nodes)):
             log_threads[conn_index]= threading.Thread(target=receiveMessage, args=(log_connected_nodes[conn_index]))
        while True:
            # Avviamo i threads, attendiamo la fine di tutti e printiamo
            for thread in log_threads:
                 thread.start()
            for thread in log_threads:
                 thread.join()     
            print("""
            ----------------------------
            Finito di eseguire i threads
            ----------------------------
            """)
                 


log_loop(int(sys.argv[1]))