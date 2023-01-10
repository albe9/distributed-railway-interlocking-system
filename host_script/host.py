import socket
import time


HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 6543  # Port to listen on (non-privileged ports are > 1023)


 # Da sceegliere la sintassi, per adesso dict che associa ad ogni nodo(Tramite RASP_ID) il nodo precedente e successivo
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
        }
    },
    2: 
    {
        1 :
        {
            "rasp_id_prev" : "0",
            "rasp_id_next" : "2",
            "prev_ip" : "127.0.0.1",
            "next_ip" : "127.0.0.1",
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


def make_config(rasp_id, routes):
    """
        Definizione protocollo : 
            -primo pacchetto:
                -Time since epoch locale,
                -Numero di route di cui fa parte il nodo
            -secondo/successivi paccehtti:
                -route_id,
                -rasp_id_prev,
                -rasp_id_next,
                -prev_ip,
                -next_ip,
    """

    config_data = []
    #primo pacchetto 
    msg = f"{int(time.time())},"
    route_count = 0
    for route_data in routes.values():
        if rasp_id in route_data:
            route_count += 1
            continue
    
    msg += f"{route_count}"
    config_data.append(msg)
    #secondo pacchetto, TODO : valutare se spezzare il pacchetto in più parti
    msg = ""
    for route_id, route_data in routes.items():
        if rasp_id in route_data:
            msg += f"{route_id}"
            for value in route_data[rasp_id].values():
                msg += ","
                msg += f"{value}"
            msg += ";"    
    config_data.append(msg)

    return config_data




def server_loop():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('0.0.0.0', PORT))       #accetta connessioni da qualsiasi indirizzo sulla porta PORT
        s.listen()
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            rasp_msg = conn.recv(1024)
            print(f"messaggio ricevuto : {rasp_msg.decode()}")

            rasp_msg = rasp_msg.decode().strip()
            rasp_id = int(rasp_msg.split("RASP_ID : ")[1])

            msg = make_config(rasp_id, Routes_test)
            conn.send(msg.encode())
        
            
        s.close()

def main():
    print(make_config(1,Routes_test))

if __name__ == "__main__":
    main()

