#!/bin/bash

while read target;
    do
            gnome-terminal --tab -- bash -c "( echo \"cmd\" ; sleep 2 ; echo \"more /usr/log/log.txt\" ; sleep 2 ) | telnet $target; bash"
        
    done < target.txt