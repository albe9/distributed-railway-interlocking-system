host_ip=$( grep -Po '(?<=\[Host_ip\] : ")[^"]*' ./build.config )

# Se lo script viene chiamato con un argomento da linea di comando,
# esso viene passatto all'host e indica il numero di richieste automatiche da fare su ogni tratta
if [ "$#" -eq 1 ]; then
    python3 ./../host_script/host.py 5 $host_ip $1
else
    python3 ./../host_script/host.py 5 $host_ip
fi
