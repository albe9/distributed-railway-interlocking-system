#!/bin/bash

path_from_wrdgb="./../../../../../../distributed-railway-interlocking-system/connect"

while read target;
    do
        ping -c2 $target > /dev/null 2>&1 &
        if [ $? -eq 0 ]; 
        then
            #crea una named pipe per ogni target su cui potremmo collegarci ed inviare comandi 
            if [ ! -p "/tmp/fifo_$target" ];
            then 
                mkfifo /tmp/fifo_$target
            fi
            #apre wrdbg in background e definisce la named pipe come stdin e un file di log come std output e std error
            (cd ./../../wrsdk-vxworks7-raspberrypi4b/tools/debug/22.03/x86_64-linux2/bin ; stdbuf -o0 ./wrdbg < /tmp/fifo_$target > $path_from_wrdgb/log_files/log_$target.txt 2>&1) &
            
            #matengo la scrittura sulla pipe sempre aperta grazie al fd 3
            exec 3>/tmp/fifo_$target

            echo "target connect vxworks7:TCP:$target:1534 -kernel $path_from_wrdgb/vxWorks" > /tmp/fifo_$target


            gnome-terminal --tab -- bash -c "telnet $target; bash"
        else 
            "Host $target not connected"
        fi
    done < target.txt





echo -e "Premi qualsiasi tasto per chiudere le connessioni\n"
read -r -s -n 1 

cleanup(){
    #chiudo la scrittura sulla pipe
    exec 3>&-
    while read target;
        do
            rm /tmp/fifo_$target
        done < target.txt
}


trap cleanup EXIT


