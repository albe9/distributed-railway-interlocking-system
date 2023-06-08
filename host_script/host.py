import socket
import time
import sys
from threading import Thread
import select
import re


PORT = 6543  # Port to listen on (non-privileged ports are > 1023)
# HOST_IP_SUFFIX = "172.23.78."
HOST_IP_SUFFIX = "192.168.1.21"
HOST_ID = 0
HOST_IP = sys.argv[2]
TAIL_ID = -9999

class Node:
    # rasp_id deve essere univoco in tutto il grafo
    # per costruire l'ip del nodo si utilizza l'ip dell'host
    # nel caso di id 0 o -9999 si costruisce un nodo host o tail
    # TODO: controllare in fase di costruzione l'unicità del rasp_id
    def __init__(self, rasp_id: int):
        if rasp_id == 0:
            self.rasp_id = rasp_id
            self.rasp_ip: str = HOST_IP
        elif rasp_id is not TAIL_ID:
            self.rasp_id = rasp_id
            self.rasp_ip: str = HOST_IP_SUFFIX + f"{rasp_id}"
        elif rasp_id == TAIL_ID:
            self.rasp_id = TAIL_ID
            self.rasp_ip = TAIL_ID
        else:
            print("Qualcosa è andato storto in fase di creazione del nodo, rasp_id errato...")

class Route:
    # route_id deve essere univoco in tutto il grafo
    # TODO: controllare in fase di construzione l'unicità del route_id e che non ci siano  magari nodi doppi in una route inseriti per sbaglio
    def __init__(self, route_id: int, ordered_list_of_nodes: list[Node]):
        self.route_id = route_id
        # Aggiungi alla lista dei nodi il nodo host in posizione 0 e nodo tail in ultima posizione
        self.nodes_list = ordered_list_of_nodes
        self.nodes_list.insert(0, Node(HOST_ID))
        self.nodes_list.insert(len(self.nodes_list), Node(TAIL_ID))
    def get_node_info(self, wanted_rasp_id: int):
        # TODO: aggiungere controlli che il nodo wanted_rasp_id esista all'interno della route
        # Cerchiamo l'indice del nodo voluto in questa route
        index_in_this_route = None
        for node in self.nodes_list:
            if node.rasp_id == wanted_rasp_id:
                index_in_this_route = self.nodes_list.index(node)
                break
        if index_in_this_route:
            # TODO: aggiungere controllo di out of range nel caso venga chiesto il nodo 0    
            # Se presente il nodo nella rotta otteniamo informazioni sul nodo precedente e successivo su quella rotta       
            prev_node_ip = self.nodes_list[index_in_this_route - 1].rasp_ip
            prev_node_id = self.nodes_list[index_in_this_route - 1].rasp_id
            next_node_ip = self.nodes_list[index_in_this_route + 1].rasp_ip
            next_node_id = self.nodes_list[index_in_this_route + 1].rasp_id
            return (prev_node_ip, prev_node_id, next_node_ip, next_node_id)
        else:
            # Se il nodo non è presente nella rotta torniamo None
            return None

class Graph:
    def __init__(self, unordered_list_of_routes: list[Route]):
        # routes è un dizionario con chiave l'id della rotta e come valore la lista di nodi di quella rotta
        self.routes = unordered_list_of_routes
        
    def query_node_data(self, wanted_rasp_id: int) -> dict:
        """
            Restituisce un dict con tutti i collegamenti presenti nel grafo relativi ad un nodo (anche su rotte diverse):

                -"prev_node_ips" : lista di ip precedenti,
                -"prev_node_ids" : lista di id precedenti,
                -"next_node_ips" : lista di ip successivi,
                -"next_node_ids" : lista di id successivi,
                -"routes" : lista di tre elementi [a,b,c] così strutturata:

                    -a id della rotta,
                    -b id del nodo precedente nella rotta a,
                    -c id del nodo successivo nella rotta a,
            
        """ 
        node_data = \
            {
                "prev_node" : dict(),
                "next_node" : dict(),
                "routes" : []
           }
        
        for route in self.routes:
            # Controlliamo che quella rotta abbia quel nodo e se lo ha salviamo le info
            node_info = route.get_node_info(wanted_rasp_id=wanted_rasp_id)       
            if node_info:
                node_data["prev_node"][node_info[1]] = node_info[0]
                node_data["next_node"][node_info[3]] = node_info[2]
                node_data["routes"].append([route.route_id, node_info[1], node_info[3]])

        return node_data
    
    def get_first_nodes(self) -> set:
        first_nodes = set()

        #per costruzione ogni route inizia con il nodo 0 (host) seguito dal primo nodo
        for route in self.routes:
            first_nodes.add(route.nodes_list[1].rasp_id)
        
        return first_nodes

def reading_and_answer_ping(fds):

    print("[T1] Reading and answer thread avviato")
    poll_obj = select.poll()
    total_conn = 0
    for fd in fds:
        poll_obj.register(fd, select.POLLIN)
        total_conn +=1

    while True:
        if total_conn == 0:
            break
        poll_result = poll_obj.poll()
        for fd_number, _ in poll_result:
            for fd in fds:
                if fd_number == fd.fileno():
                    msg = fd.recv(50)
                    if msg.decode() == "":
                        print(f"Ip {fd.getpeername()} Disconnesso")
                        poll_obj.unregister(fd)
                        fd.close()
                        total_conn -= 1
                    else:
                        if (msg.decode() == "PING_REQ;."):
                            ack_msg = "PING_ACK;."
                            fd.send(ack_msg.encode())
                            # print(f"[T1] Inviato a {fd.getpeername()} : {ack_msg}")
                        else:                      
                            print(f"[T1] Messaggio da {fd.getpeername()} : {msg.decode()}")
    print("Tutti i nodi disconnessi, reading thread terminato")
                    
def send_msg_from_keyboard(connected_nodes:list[socket.socket]):
    print("[T2] Send msg from keyboard thread avviato")
    while True:
        msg = input("[T2] Messaggio da inviare a tutti i nodi:\n")
        # Controlliamo che la sintassi del messaggio che vogliamo inviare sia corretta
        if check_syntax(msg):
            for conn in connected_nodes:
                # Inviamo il messaggio a tutti i nodi connessi 
                conn.send(msg.encode())
                print("[T2] Messaggio inviato")
        else:
            print("Errore nella sintassi del messaggio, messaggio non inviato")
        # Attendi 5 secondi prima di poter inviare un altro messaggio [senza lo sleep 
        # avremo una sovrapposizione di output grafico dovuta alle risposte a questo send]
        time.sleep(5)

    
    

# I route sono dizionari con chiave id rotta e con valore un dizionario con chiave id del nodo e come valore informazioni del nodo nella rotta
# rasp_id 0 è stato assegnato all'host che ha ip 172.23.78.0
# rasp_id -9999 è stato assegnato al nodo [che non esiste] successivo al capolinea ed esso avrà ip -9999
# Gli ip sono stati assegnati a partire da quello dell'host, ad esempio il rasp 1 ha ip [172.23.78].1, il rasp 2 ha [172.23.78].2
# Per visualizzare queste rotte accedere a ProgettoSWE4ES>Diagrammi>Routes


def make_config_string(rasp_id, node_data):
    """
        Definizione protocollo : 
            -primo pacchetto:
                -Time since epoch locale,
                -Numero di nodi precedenti,
                -Numero di nodi successivi,
                -Numero di route di cui fa parte il nodo,
            -secondo pacchetto:
                -prev_node_ips,
                -prev_node_ids,
            -terzo pacchetto:
                -next_node_ips,
                -next_node_ids,
            -quarto pacchetto e successivi:
                -route_id,
                -rasp_id_prev,
                -rasp_id_next,

        Separatori : 
        ';' tra due tipologie di pacchetto differenti
        '/' tra due pacchetti dello stesso tipo (accade solo con i pacchetti di tipo 4)
        ',' tra i dati dello stesso pacchetto        
        
    """

    config_string = ""
    #primo pacchetto 
    #TODO controllare ora legale
    config_string += f"{int(time.mktime(time.localtime())) +  7200},"
    config_string += f"{len(node_data['prev_node'])},"
    config_string += f"{len(node_data['next_node'])},"
    config_string += f"{len(node_data['routes'])};"

    #secondo pacchetto
    ip_str = ""
    id_str = ""
    for id,ip in node_data['prev_node'].items():
        id_str += f"{id},"
        ip_str += f"{ip},"

    id_str = id_str.removesuffix(',')
    id_str += ";" 
    config_string += ip_str + id_str
    
    # #terzo pacchetto
    ip_str = ""
    id_str = ""
    for id,ip in node_data['next_node'].items():
        id_str += f"{id},"
        ip_str += f"{ip},"

    id_str = id_str.removesuffix(',')
    id_str += ";" 
    config_string += ip_str + id_str 

    # #quarto pacchetto e successivi
    for route_id, prev_id, next_id in node_data['routes']:
        config_string += f"{route_id},{prev_id},{next_id}/"
    config_string = config_string.removesuffix('/')
    config_string += ";"

    return config_string


def check_syntax(msg_to_check:str):
    # Il messaggio per matchare deve essere PING; oppure REQ;numero;numero
    pattern = r'^(PING;\.|REQ;\d+;\d+\.)$'
    return re.match(pattern, msg_to_check) is not None


def server_loop(node_num, net_graph):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST_IP, PORT))       #accetta connessioni sull'indirizzo HOST_IP e porta PORT
        # s.bind(("0.0.0.0", PORT))
        s.listen()
        #aspetto la connessione di ogni nodo nella rete per inviare
        connected_nodes = []
        for node_idx in range(node_num):
            conn, addr = s.accept()
            connected_nodes.append(conn)
            print(f"Connected by {addr}")
            rasp_msg = conn.recv(1024)
            print(f"[MSG] : {rasp_msg.decode()}")

            rasp_msg = rasp_msg.decode().strip()
            rasp_id = int(rasp_msg.split("RASP_ID : ")[1])

            msg = make_config_string(rasp_id, net_graph.query_node_data(rasp_id))
            conn.send(msg.encode())

        #Controllo che tutti i nodi della rete abbiano effettuato la configurazione
        for conn in connected_nodes:
            rasp_msg = conn.recv(1024)  
            print(f"[MSG] : {rasp_msg.decode()}")

        #Avverto tutti i nodi che possono iniziare a connettersi in catena
        for conn in connected_nodes:
            conn.send("Avvio connessioni".encode())
            conn.close()
        
        connected_nodes.clear()
        #attendo le connessioni dai nodi direttamente collegati all'host (i primi di ogni root)
        first_nodes = net_graph.get_first_nodes()
        for node_id in first_nodes:
            conn, addr = s.accept()
            connected_nodes.append(conn)
            #TODO definire protocollo prima connessione, per adesso rispondo con ip dell'host
            print(f"Connected by {addr}")
            msg = conn.recv(20)
            print(f"[MSG] : {msg.decode()}")
            conn.send("RASP_ID : 0".encode())

        #attendo il messaggio dai nodi con la conferma che tutte le connessioni sono state stabilite
        for conn in connected_nodes:
            msg = conn.recv(20)
            print(f"[MSG] : {msg.decode()}")


        print("Tutte le connessioni sono state stabilite")

        # Genero ed avvio un thread che fa il polling e stampa i messaggi ricevuti dai nodi
        reading_thread = Thread(target=reading_and_answer_ping, args=(connected_nodes,))
        reading_thread.start()

        # Genero ed avvio un thread che permette di inviare un messaggio verso tutti i nodi
        send_msg_thread = Thread(target=send_msg_from_keyboard, args=(connected_nodes,))
        send_msg_thread.start()
                    

node1 = Node(1)
node2 = Node(2)
node3 = Node(3)
node4 = Node(4)
node5 = Node(5)
node6 = Node(6)
node7 = Node(7)

graph_testing = Graph([Route(1, [node1, node2, node3])])
graph_testing2 = Graph([Route(1, [node1, node2, node3, node4]), Route(2, [node1, node2, node5, node4])])
graph_testing3 = Graph([Route(1, [node1, node2, node3]), Route(2, [node4, node2, node5])])
graph_testing4 = Graph([Route(1, [node1, node2, node3]), 
                        Route(2, [node4, node2, node5]), 
                        Route(3, [node6, node2, node7])])

def main():
    server_loop(int(sys.argv[1]), graph_testing3)
    # print(sys.argv[1], sys.argv[2])

if __name__ == "__main__":
    main()

