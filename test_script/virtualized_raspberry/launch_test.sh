#!/bin/bash

#base ip addr : 172.23.78.0
N_IP=2
#aggiungo N_IP all'interfaccia di rete eth0
for ((label=1; label<=$N_IP; label++))
do
    ip=$(ip address show dev eth0 label eth0:$label)
    if [ -z "$ip" ]
    then
        sudo ip address add 172.23.78.$label/24 dev eth0 label eth0:$label
    fi
done

# #cambia directory e compila 
(cd ../../build && make)

# #esegue in parallelo i file compilati con i relativi args
../../build/Virtualized_raspberry "172.23.78.1" 1 &
../../build/Virtualized_raspberry "172.23.78.2" 2