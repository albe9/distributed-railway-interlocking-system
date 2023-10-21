import socket
import time
import sys
import threading
import subprocess
import os
import signal
import re
import codecs

LOG_PORT = 6544  # Port to listen on (non-privileged ports are > 1023)
HOST_IP = sys.argv[2]
WD_PATH = sys.argv[3]

# HOST_IP = "192.168.1.202"
# WD_PATH = f"{os.path.abspath(os.path.join(os.getcwd(), os.pardir))}/connect"


def receiveAndSaveLog(conn : socket.socket, addr : tuple[str, int]):

    prefix = f"[{addr[0][-1]}]"
    print(f"{prefix} Avvio thread ricezione da ip : {addr}")

    log = ""
    log_file_size = 0
    flag_start_sending = False
    # comunicazioni iniziali con il nodo
    while not flag_start_sending:
        msgs = conn.recv(1024).decode(encoding='utf8', errors="costum_handler").split('.')
        for msg in msgs:
            if msg == "":
                pass
            else:
                # se ho ricevuto nello stesso conn.recv sia START_SENDING che una parte del log effettivo, lo salvo direttamente sulla variabile log
                if flag_start_sending:
                    log += msg
                else:
                    print(f"{prefix} {msg}")

                    match_size = re.search("\[(\d+)\] bytes", msg)
                    match_start = re.search("\[START_SENDING\]", msg)

                    if match_size != None :
                        log_file_size = int(match_size.group(1))
                    elif match_start != None:
                        flag_start_sending = True
    
    # ricevi tutt i messaggi in arrivo da un nodo e lo unisco
    rc_buff_size = 4097 
    last_progress = 0
    actual_progress = 0
    encoded_log = b''

    while True:        
        # ricevo il chunk di bytes e rimuovo i suffissi                
        recived_bytes = conn.recv(rc_buff_size)
        encoded_log += recived_bytes

        # log_data = recived_bytes.decode(encoding='utf8', errors="costum_handler")
        # log += log_data


        if log_file_size != 0:
            actual_progress = round((len(encoded_log) * 100 ) / log_file_size)
            #se la differenza in percentuale è maggiore di 5 stampo un feedback
            if actual_progress - last_progress >= 10:
                print(f"{prefix} {actual_progress}%")
                last_progress = actual_progress
        # Finito di ricevere salvo il log ottenuto in un file
        flag_errors_in_log = False
        if recived_bytes == b'':  
            conn.close()   
            
            encoded_log = bytearray(encoded_log)
            
            # Define the chunk size and number of bytes to skip
            chunk_size = 4131
            bytes_to_skip = 34
    
            encoded_clean_log = bytearray()

            # Iterate through the original bytearray
            for i in range(0, len(encoded_log), chunk_size):
                chunk = encoded_log[i:i+chunk_size]
                encoded_clean_log.extend(chunk[:chunk_size - bytes_to_skip])
                
            # # Rimuovi i bytes nel renge 00 to 1F
            # encoded_clean_log = bytearray(byte for byte in encoded_log if not ((0x00 <= byte <= 0x1F) and byte not in (0x0A, 0x0D)))
            # # Altri bytes specifici da rimuovere
            # bytes_to_remove_list = [b"\xee", b"\xf6"]
            # for byte_to_remove in bytes_to_remove_list:
            #     encoded_clean_log = encoded_clean_log.replace(byte_to_remove, b"")
            
            with open(f"{WD_PATH}/execution_log_files/log_{addr[0]}.txt", "w") as log_file:
                decoded_log = encoded_clean_log.decode(errors="costum_handler")

                if decoded_log.endswith("[END_WITH_SUCCESS]"):
                    decoded_log = decoded_log.removesuffix("[END_WITH_SUCCESS]")
                    # cleaning dell'ultima parte del log
                    last_msg = "Avvio task di trasferimento log verso l'host"
                    last_index = decoded_log.rfind(last_msg)
                    decoded_log = decoded_log[:last_index + len(last_msg)]
                else:
                    flag_errors_in_log = True
                # Scrivo i log 
                log_file.write(decoded_log)

                if flag_errors_in_log:
                    print(f"{prefix} Errore nel log, salvato log_{addr[0]}.txt") 
                else:
                    print(f"{prefix} Salvato log_{addr[0]}.txt") 
            break

    
# Creiamo una lista di thread
log_threads = []   
# Creiamo una lista e aggiungiamo tutti i nodi che devono loggare
log_connected_nodes : list[tuple[socket.socket, tuple[str, int]]] = []


def log_loop(n_nodes:int):
    # TODO: trovare un modo per controllare se è già attivo un processo precedente di log che utilizza le porte
    # check_if_log_active()
    # time.sleep(2)
    print("Avvio server ricezione log")

    # Usando with si ha che alla fine dell'esecuzione NON dobbiamo chiamare s.close()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST_IP, LOG_PORT))       #accetta connessioni sull'indirizzo HOST_IP e porta PORT
        s.listen()
        # Creiamo una lista e aggiungiamo tutti i nodi che devono loggare
        global log_connected_nodes 
        for node_index in range(n_nodes):           
            log_conn, log_addr = s.accept()
            log_connected_nodes.append((log_conn, log_addr))
            print("Connesso : ", log_addr)
        # Controlliamo che sia arrivata qualche comunicazione
        if log_connected_nodes:
            # recuperiamo la lista ed aggiungiamo un thread per ogni socket
            global log_threads
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

def handler_ctrlc(signum, frame):
    global log_threads
    global log_connected_nodes
    for thread in log_threads:
        thread.join(0)
    for conn, addr in log_connected_nodes:
        conn.close()
    sys.exit()

def utf8_errors_handler(err):
    # Faccio il replace con "" dei caratteri non utf-8
    return("", err.end)

def main():
    signal.signal(signal.SIGINT, handler_ctrlc)
    codecs.register_error("costum_handler", utf8_errors_handler)

    log_loop(int(sys.argv[1]))
   

if __name__ == "__main__":
    main()
