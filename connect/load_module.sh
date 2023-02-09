#!/bin/sh

cd ./../../wrsdk-vxworks7-raspberrypi4b/tools/debug/22.03/x86_64-linux2/bin

while read target;
    do
        ping -c2 $target > /dev/null 2>&1 &
        if [ $? -eq 0 ]; 
        then
            echo "module load " > /tmp/fifo_$target
        else 
            "Host $target not connected"
        fi
    done < target.txt