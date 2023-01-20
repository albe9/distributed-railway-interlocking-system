import socket

HOST = "172.23.78.1"  # The server's hostname or IP address
PORT = 6543  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.send("Connessione da raspberry con RASP_ID : 0".encode())
    # s.recv(1024)
    
    s.close()
