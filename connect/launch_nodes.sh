host_ip=$( grep -Po '(?<=\[Host_ip\] : ")[^"]*' ./build.config )

while read target;
    do
        ping -c2 $target > /dev/null 2>&1
        if [ $? -eq 0 ]; 
        then
            # Se c'è un argomento ed è SIM allora avvio i rasp in modalità sensorOn/Off simulata
            if [ "$#" -eq 1 ]; then
                if [ "$1" = "SIM" ]; then
                    ( sleep 3 ; echo "startInit(${target: -1}, \"$host_ip\", 1)" ; sleep 3 ) | telnet $target &
                else
                    echo "Argomento errato"
                fi
            else
                ( sleep 3 ; echo "startInit(${target: -1}, \"$host_ip\", 0)" ; sleep 3 ) | telnet $target &
            fi
        else 
            echo "Host $target not connected"
        fi
    done < target.txt


