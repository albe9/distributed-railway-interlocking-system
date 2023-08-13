import socket
import time
import sys
import threading
import subprocess
import os
import signal

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

# Controlliamo se esiste un processo log.py attivo e nel caso terminiamolo 
def check_if_log_active():
    # Esegui il comando ps -fA [ps mostra i processi in esecuzione -f info complete e A di tutti gli utenti]
    # e l'ouput viene reinderizzato al grep grazie alla pipe [ | ]
    log_script_pid = os.getpid()

    # command = 'ps -fA | grep host_script/log.py'
    # result = subprocess.run(command, shell=True, capture_output=True, text=True)

    # # Controlla se il processo è attivo
    # if "log.py" in result.stdout:
    #     # Estrai l'ID del processo
    #     lines = result.stdout.strip().split('\n')
    #     process_line = [line for line in lines if "log.py" in line][0]
    #     process_id = process_line.split()[1]

    #     # Killa il processo
    #     if process_id != log_script_pid:
    #         try :
    #             os.kill(int(process_id), signal.SIGTERM)
    #             print("processo precedente terminato")
    #         except:
    #              None
    
    # Controllo se la log port è ancora in uso, nel caso la chiudo
    command = f"lsof -i -P -n | grep {LOG_PORT}"
    result = subprocess.run(command, shell=True, capture_output=True, text=True)

    

    if result.stdout != "":
        # Estrai l'ID del processo
        lines = result.stdout.strip().split('\n')
        print(lines)
        
        for line in lines:
            process_id = line.split()[1]

            if process_id != log_script_pid:
                # Killa il processo
                try :
                    os.kill(int(process_id), signal.SIGTERM)
                    print("Porta in uso precedente terminata")
                except:
                    # Può tentare di killare due volte lo stesso processo, saltiamo l'errore
                    None
    
    

def log_loop(n_nodes:int):
    # TODO: trovare un modo per controllare se è già attivo un processo precedente di log che utilizza le porte
    # check_if_log_active()
    # time.sleep(2)
    print("Avvio log")

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