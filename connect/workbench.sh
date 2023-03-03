#!/bin/bash

#salvo i target in un array
TARGETS=()
while read target;
    do
        TARGETS+=("$target")
    done < target.txt

launch_connect(){
    #verifico che la connessione ai target non sia già attiva (basta controllare la presenza delle pipe)
    for target in ${TARGETS[@]};
        do
            if [ -p "/tmp/fifo_$target" ];
            then 
                echo "Connessione già eseguita"
                return 0
            else
                #crea una named pipe per ogni target su cui potremmo collegarci ed inviare comandi 
                mkfifo /tmp/fifo_$target
            fi
            
        done

    gnome-terminal --tab -- bash -c "./connect.sh; bash"

    #itero i raspberry per verificare che le connessioni siano tutte effettuate
    for target in ${TARGETS[@]};
        do
            while true
                do  
                    sleep 0.5
                    grep 'Connected to' ./log_files/log_$target.txt
                    if [ $? -eq 0 ];
                        then
                            #resetto il log e passo al prossimo raspberry
                            > ./log_files/log_$target.txt
                            break
                        fi
                done
        done
}

build(){

    # Elimino i file generati precedentemente
    rm -R ./../Interlocking_system/rpivsb_ARMARCH8Allvm_LP64_LARGE_SMP/*

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
            grep 'Nessuna regola per generare' ./log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Riprovo il build"
                    build
                    break
                fi
            grep 'Build Failed' ./log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Build fallito"
                    stty sane
                    exit
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

    #verifico che le pipe esistano
    for target in ${TARGETS[@]};
        do
            if [ ! -p "/tmp/fifo_$target" ];
            then 
                launch_connect
                break
            fi
        done
    

    cd ./../
    module_path=$PWD/Interlocking_system/rpivsb_ARMARCH8Allvm_LP64_LARGE_SMP/Interlocking_system_partialImage/Debug/Interlocking_system_partialImage.o
    cd ./connect


    for target in ${TARGETS[@]};
        do
            ping -c2 $target > /dev/null 2>&1 &
            if [ $? -eq 0 ]; 
            then
                #invio il comando per caricare i moduli
                echo "module load $module_path" > /tmp/fifo_$target
            else 
                "Host $target not connected"
            fi
        done

    
    #itero  i log finchè tutti non hanno caricato i moduli

    start=$SECONDS
    for target in ${TARGETS[@]};
        do
            while [ true ];
                do
                    duration=$(( SECONDS - start ))
                    if [ $duration -gt 60 ];
                        then
                            echo "Problema, attesa per il load maggiore di 60 secondi, controlla i log"
                            exit
                        fi
                    sleep 1

                    grep -Pzo 'Loading module .*done\nModule.*\n\(wrdbg\)' ./log_files/log_$target.txt > /dev/null 2>&1 
                    if [ $? -eq 0 ]; 
                        then
                            echo "log_$target.txt" ha caricato
                            #resetto il log
                            > ./log_files/log_$target.txt
                            break
                        fi
                done
            
        done
}

task_delete(){

    for target in ${TARGETS[@]};
        do
            ( echo "reset" ; sleep 3 ; echo "td initTask" ; sleep 2 ; echo "td LogTask" ; sleep 2 ; echo "td wifiTask" ; sleep 2 ) | telnet $target &
        done
}

reboot_rasp(){

    for target in ${TARGETS[@]};
        do
            ( echo "cmd" ; sleep 2 ; echo "reboot -c" ; sleep 2 ) | telnet $target &
        done
    
    rm /tmp/fifo*
}

see_log(){
    gnome-terminal --tab -- bash -c "./see_log.sh; bash"
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
        -o  (output)           Mostro i log dei raspberry
        -d  (delete)           Effettua il task delete nei raspberry (per ora task_wifi)
        -r  (reboot)           Effettua il reboot dei raspberry
        -h  (help)             Mostra questo messaggio
    "

}

while getopts "cblodrh" options;
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
            o)
                echo "Mostro i log"
                see_log
                ;;
            d)
                echo "Delete dei task"
                task_delete
                ;;
            r)
                echo "Reboot dei raspberry"
                reboot_rasp
                ;;
            h)
                help_workbench
                ;;
        esac
    done