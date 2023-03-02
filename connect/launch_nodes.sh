host_ip=$( grep -Po '(?<=\[Host_ip\] : ")[^"]*' ./build.config )

while read target;
    do
        ping -c2 $target > /dev/null 2>&1
        if [ $? -eq 0 ]; 
        then
            ( echo "startInit(${target: -1}, \"$host_ip\")" ; sleep 3 ) | telnet $target &
        else 
            echo "Host $target not connected"
        fi
    done < target.txt


