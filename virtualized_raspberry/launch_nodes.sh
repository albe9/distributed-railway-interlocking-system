#!/bin/bash

if [ ! $1 ];
then 
    echo "inserire il nome dell'interfaccia (es eth0)"
    exit
fi

#wlp0s20f3
net_interface=$1

# host ip addr : 192.168.1.210
N_IP=5
#aggiungo N_IP all'interfaccia di rete $net_interface
for ((label=0; label<=$N_IP; label++))
do
    ip=$(ip address show dev $net_interface label $net_interface:$label)
    if [ -z "$ip" ]
    then
        sudo ip address add 192.168.1.21$label/24 dev $net_interface label $net_interface:$label
    fi
done

# #cambia directory e compila 
(cd ../build && make)



# esegue i file compilati con i relativi args

for ((label=1; label<=$N_IP; label++))
do
    ../build/Virtualized_raspberry "192.168.1.21$label" $label &
done

remove_ip(){

    for ((label=0; label<=$N_IP; label++))
    do
        sudo ip address del 192.168.1.21$label/24 dev $net_interface label $net_interface:$label
    done
}
