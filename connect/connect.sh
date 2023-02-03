#!/bin/sh

while read target;
    do
        ping -c2 $target &> /dev/null
        if [ $? -eq 0 ]; 
        then
            gnome-terminal --tab --command "telnet $target"
        else 
            "Host $target not connected"
        fi
    done < target.txt



