import socket
import time
import sys
import threading

LOG_PORT = 6544  # Port to listen on (non-privileged ports are > 1023)
HOST_IP = sys.argv[2]
WD_PATH = sys.argv[3]

def receiveAndSaveLog(conn : socket.socket, addr : tuple[str, int]):
    # ricevi tutto il messaggio in arrivo da un nodo e lo unisco
     log = "" 
     while True:                        
            log_data = conn.recv(1024).decode()
            log += log_data
            # Finito di ricevere salvo il log ottenuto in un file
            if not log_data:            
                with open(f"{WD_PATH}/execution_log_files/log_{addr[0]}.txt", "w") as log_file:
                    log_file.write(log)
                    log_file.close()
                print(f"Salvato log_{addr[0]}.txt")    
                break

def log_loop(n_nodes:int):
    # Usando with si ha che alla fine dell'esecuzione NON dobbiamo chiamare s.close()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST_IP, LOG_PORT))       #accetta connessioni sull'indirizzo HOST_IP e porta PORT
        s.listen()
        # Creiamo una lista e aggiungiamo tutti i nodi che devono loggare
        log_connected_nodes : list[tuple[socket.socket, tuple[str, int]]] = []
        for node_index in range(n_nodes):           
            log_conn, log_addr = s.accept()
            log_connected_nodes.append((log_conn, log_addr))
        # Controlliamo che sia arrivata qualche comunicazione
        if log_connected_nodes:
            # Creiamo una lista di thread, uno per ogni socket
            log_threads = []
            for conn_index in range(len(log_connected_nodes)):
                log_threads.append(threading.Thread(target=receiveAndSaveLog, args=(log_connected_nodes[conn_index][0], log_connected_nodes[conn_index][1])))
            # Avviamo i threads e attendiamo la fine di tutti
            for thread in log_threads:
                    thread.start()
            for thread in log_threads:
                    thread.join()   
            print("----------------------------")
            print("Threads chiusi -> Finito di eseguire il log")
            print("----------------------------")
        # Altrimenti nessun nodo si è connesso, nonostante la funzione accept sia bloccante...
        else:
            print("Non è stato ricevuto nessun messaggio, qualcosa è andato storto...")
                
def main():
    log_loop(int(sys.argv[1]))


if __name__ == "__main__":
    main()