while read target;
    do
        ping -c2 $target > /dev/null 2>&1
        if [ $? -eq 0 ]; 
        then
            echo "startInit(${target: -1})" | telnet $target &
        else 
            echo "Host $target not connected"
        fi
    done < target.txt