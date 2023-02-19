#!/bin/bash


launch_connect(){
    #verifico che la connessione ai target non sia già attiva (basta controllare la presenza delle pipe)
    while read target;
        do
            if [ -p "/tmp/fifo_$target" ];
            then 
                echo "Connessione già eseguita"
                return 0
            fi
        done < target.txt

    gnome-terminal --tab -- bash -c "./connect.sh; bash"
}

build(){

    WINDRIVER_PATH=$(grep -Po '(?<=\[WindRiver_path\] : ")[^"]*' ./build.config)

    if [ ! -p "/tmp/fifo_wrtool" ];
    then 
        mkfifo /tmp/fifo_wrtool
    fi
    #lancio la shell wrtool definendo il workspace
    $WINDRIVER_PATH/workbench-4/wrtool -data ./../../ < /tmp/fifo_wrtool > ./log_files/build_log.txt &
    echo "cd .." > /tmp/fifo_wrtool
    echo "prj import Interlocking_system" > /tmp/fifo_wrtool
    echo "cd ./Interlocking_system" > /tmp/fifo_wrtool
    echo "prj build" > /tmp/fifo_wrtool


    while true
        do  
            sleep 1
            grep 'Build Failed' ./log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Build fallito"
                    break
                fi
            grep 'Build Finished' ./log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Build completato correttamente"
                    #resetto il build_log
                    > ./log_files/build_log.txt
                    break
                fi
        done
    
    #non so perchè ma dopo il while loop lo stato del terminale è "weird" non si vede l'input da tastiera
    stty sane
}

load_module(){

    #verifico che la connessione ai target sia attiva (basta controllare la presenza delle pipe)
    while read target;
        do
            if [ ! -p "/tmp/fifo_$target" ];
            then 
                launch_connect
                break
            fi
        done < target.txt
    

    cd ./../
    module_path=$PWD/Interlocking_system/rpivsb_ARMARCH8Allvm_LP64_LARGE_SMP/Interlocking_system_partialImage/Debug/Interlocking_system_partialImage.o
    cd ./connect

    logs=()
    while read target;
        do
            ping -c2 $target > /dev/null 2>&1 &
            if [ $? -eq 0 ]; 
            then
                #salvo i file di log in un array
                logs+=("log_$target.txt")
                #invio il comando per caricare i moduli
                echo "module load $module_path" > /tmp/fifo_$target
            else 
                "Host $target not connected"
            fi
        done < target.txt

    #itero  i log finchè tutti non hanno caricato i moduli
    loading=${#logs[@]}
    while [ $loading -gt 0 ];
        do
            sleep 1
            for log in ${logs[@]};
                do
                    grep -Pzo 'Loading module .*done\nModule.*\n\(wrdbg\)' ./log_files/$log > /dev/null 2>&1 
                    if [ $? -eq 0 ]; 
                        then
                            echo $log ha caricato
                            loading=$(( $loading - 1 ))
                            > ./log_files/$log
                        fi
                done
        done
}

help_workbench(){

echo -e "Requisiti:
    -rpivsb            Estrarre l'archivio rpivsb.rar nella root del git
    -vxWorks           Copiare il file vxWorks da onedrive nella cartella /connect/
    -build.config      Copiare il file build_example.config e rinominarlo in build.config modificando le voci opportunamente
    "
echo -e "Opzioni: 
        -c  (compile)          Si connette ai raspberry e apre le shell telnet per eseguire i comandi in remoto
        -b  (build)            Esegue il Build del progetto, in caso di errori salva lo stato in log_files/build_log.txt
        -l  (load)             Esegue il Load del modulo sui raspberry
        -h  (help)             Mostra questo messaggio
    "

}

while getopts "cblh" options;
    do                                  
        case "${options}" in      
            c)
                echo "Eseguo la connessione ai target"
                launch_connect
                ;;    
            b)
                echo "Eseguo il build del progetto"
                build
                ;;
            l)
                echo "Eseguo il load del modulo"
                load_module
                ;;
            h)
                help_workbench
                ;;
        esac
    done