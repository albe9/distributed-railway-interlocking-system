import socket
import time


HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 6543  # Port to listen on (non-privileged ports are > 1023)


 # Da sceegliere la sintassi, per adesso dict che associa ad ogni nodo(Tramite RASP_ID) il nodo precedente e successivo
Route_test =  \
{
    1 :
    {
        "prev_ip" : "127.0.0.1",
        "next_ip" : "0.0.0.0",
    },
    2 :
    {
        "prev_ip" : "127.0.0.1",
        "next_ip" : "0.0.0.0",
    }
}



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

        #invio il time since epoch per poter settare data e ora sul raspberry
        msg = f"time:{int(time.time())}"
        conn.send(msg.encode())

        for key,value in Route_test[rasp_id].items():
            msg = key + ":" + value
            conn.send(msg.encode())
       
        
    s.close()



