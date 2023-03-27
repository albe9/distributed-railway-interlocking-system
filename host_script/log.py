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
        # s.bind(("0.0.0.0", PORT))
        s.listen()
        log_conn, log_addr = s.accept()
        while True:
            # ricevi e printa tutto il messaggio
            log_data = log_conn.recv(1024).decode()
            print(log_data)
            if not log_data:
                break


log_loop(int(sys.argv[1]))