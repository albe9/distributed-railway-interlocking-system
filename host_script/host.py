import socket
import time


HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 6543  # Port to listen on (non-privileged ports are > 1023)


 # Da sceegliere la sintassi, per adesso dict che associa ad ogni nodo(Tramite RASP_ID) il nodo precedente e successivo
Routes_test =  \
{
    "a":
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
    }
}


def make_config(rasp_id, routes):
    data = f"Time:{int(time.time())}"
    for route_name, route_data in routes.items():
        data += ","
        data += f"Route:{route_name}"
        for key,value in route_data[rasp_id].items():
            data += ","
            data += key + ":" + value
        data += ","    
        data += "Stop:1"
    return data




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



