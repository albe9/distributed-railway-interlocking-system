#!/bin/bash

cd ./../../wrsdk-vxworks7-raspberrypi4b/tools/debug/22.03/x86_64-linux2/bin

#crea una named pipe su cui potremmo collegarci ed inviare comandi 
if [ ! -p "/tmp/fifo_test" ];
then 
    mkfifo /tmp/fifo_test
fi

#apre wrdbg in background e definisce la named pipi come stdin

./wrdbg < /tmp/fifo_test &
#matengo la scrittura sulla pipe sempre aperta grazie al fd 3
exec 3>/tmp/fifo_test

echo -e "Premi qualsiasi tasto per chiudere le connessioni\n"
read -r -s -n 1 
#chiudo la scrittura sulla pipe
exec 3>&-

echo Terminato