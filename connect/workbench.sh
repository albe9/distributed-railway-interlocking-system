#!/bin/bash

#salvo i target in un array
TARGETS=()
while read target;
    do
        TARGETS+=("$target")
    done < target.txt

# Controllo se è presente la directory in cui salvare i log, altrimenti la creo
if [ ! -d "./workbench_log_files" ];
    then 
        mkdir ./workbench_log_files
    fi

launch_connect(){

    #verifico che la connessione ai target non sia già attiva (basta controllare la presenza delle pipe)
    for target in ${TARGETS[@]};
        do
            if [ -p "/tmp/fifo_$target" ];
            then 
                rm /tmp/fifo_$target
            fi
            
            #crea una named pipe per ogni target su cui potremmo collegarci ed inviare comandi 
            mkfifo /tmp/fifo_$target
            
        done

    gnome-terminal --tab -- bash -c "./connect.sh; bash"

    #itero i raspberry per verificare che le connessioni siano tutte effettuate

    start=$SECONDS
    rasp_counter=0

    while [ true ];
    do
        duration=$(( SECONDS - start ))
        if [ $duration -gt 120 ];
        then
            echo "Problema, attesa per la connessione di 120 secondi, controlla i log"
            exit
        fi
        for target in ${TARGETS[@]};
        do
            if grep -Pzoq 'Connected to' ./workbench_log_files/log_$target.txt > /dev/null 2>&1;  
            then
                echo "log_$target.txt connesso" 
                ((rasp_counter++))
                #resetto il log
                > ./workbench_log_files/log_$target.txt
            fi

            if [ "$rasp_counter" -eq "${#TARGETS[@]}" ]; 
            then
                break 2
            fi
        done
        sleep 1 
    done
}

build(){

    #resetto il build_log
    > ./workbench_log_files/build_log.txt

    # Elimino i file generati precedentemente
    rm -R ./../Interlocking_system/rpivsb_ARMARCH8Allvm_LP64_LARGE_SMP/*

    WINDRIVER_PATH=$(grep -Po '(?<=\[WindRiver_path\] : ")[^"]*' ./build.config)

    # Controllo se è presente la pipe in cui scrivere le istruzioni, altrimenti la creo
    if [ ! -p "/tmp/fifo_wrtool" ];
    then 
        mkfifo /tmp/fifo_wrtool
    fi

    if [ ! -d "./workbench_log_files" ];
    then 
        mkdir ./workbench_log_files
    fi

    #lancio la shell wrtool definendo il workspace
    $WINDRIVER_PATH/workbench-4/wrtool -data ./../../ < /tmp/fifo_wrtool > ./workbench_log_files/build_log.txt &
    sleep 0.5
    echo "cd .." > /tmp/fifo_wrtool
    sleep 0.5
    echo "prj import Interlocking_system" > /tmp/fifo_wrtool
    sleep 0.5
    echo "cd ./Interlocking_system" > /tmp/fifo_wrtool
    sleep 0.5
    echo "prj build" > /tmp/fifo_wrtool

    while true
        do  
            sleep 1
            grep 'Nessuna regola per generare' ./workbench_log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Riprovo il build"
                    build
                    break
                fi
            grep 'No rule to make target' ./workbench_log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Riprovo il build"
                    build
                    break
                fi
            grep 'Build Failed' ./workbench_log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Build fallito"
                    stty sane
                    exit
                fi
            grep 'Build Finished' ./workbench_log_files/build_log.txt
            if [ $? -eq 0 ];
                then
                    echo "Build completato correttamente"
                    break
                fi
        done
    
    #non so perchè ma dopo il while loop lo stato del terminale è "weird" non si vede l'input da tastiera
    stty sane
}

load_module(){
    # Eseguo il delete dei task (a prescindere tanto non genera errori)
    echo "Eseguo il delete dei task (a prescindere tanto non genera errori)"
    task_delete
    # Eseguo l'unload prima di ricaricare i moduli
    echo "Tento di rimuovere i moduli precedenti"
    unload_module

    echo "Procedo con il load dei moduli"


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
    rasp_counter=0

    while [ true ];
    do
        duration=$(( SECONDS - start ))
        if [ $duration -gt 120 ];
        then
            echo "Problema, attesa per il load maggiore di 120 secondi, controlla i log"
            exit
        fi
        for target in ${TARGETS[@]};
        do
            if grep -Pzoq 'Loading module .*done\nModule.*\n\(wrdbg\)' ./workbench_log_files/log_$target.txt > /dev/null 2>&1; 
            then
                echo "log_$target.txt ha eseguito il load correttamente"
                ((rasp_counter++))
                #resetto il log
                > ./workbench_log_files/log_$target.txt
            fi

            if [ "$rasp_counter" -eq "${#TARGETS[@]}" ]; 
            then
                break 2
            fi
        done
        sleep 1 
    done

}

unload_module(){
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
                echo "module unload $module_path" > /tmp/fifo_$target
            else 
                "Host $target not connected"
            fi
        done

    #itero  i log finchè tutti non hanno caricato i moduli

    start=$SECONDS
    rasp_counter=0
   
    while [ true ];
    do
        duration=$(( SECONDS - start ))
        if [ $duration -gt 120 ];
        then
            echo "Problema, attesa per l'unload maggiore di 120 secondi, controlla i log"
            exit
        fi
        for target in ${TARGETS[@]};
        do
            if grep -Pzoq 'Unloading module .*done' ./workbench_log_files/log_$target.txt > /dev/null 2>&1; 
            then
                echo "log_$target.txt ha eseguito l'unload correttamente"
                ((rasp_counter++))
                #resetto il log
                > ./workbench_log_files/log_$target.txt
            
            elif grep -Pzoq 'error: bad module ID or name' ./workbench_log_files/log_$target.txt > /dev/null 2>&1; 
            then
                echo "Impossibile eseguire l'unload, modulo già rimosso o non ancora caricato log_$target.txt"
                ((rasp_counter++))
                #resetto il log
                > ./workbench_log_files/log_$target.txt
            elif grep -Pzoq 'error' ./workbench_log_files/log_$target.txt > /dev/null 2>&1; 
            then
                echo "Impossibile eseguire l'unload, il modulo ha risorse aperte log_$target.txt"
                ((rasp_counter++))
                #resetto il log
                > ./workbench_log_files/log_$target.txt
            fi
            
            if [ "$rasp_counter" -eq "${#TARGETS[@]}" ]; 
            then
                break 2
            fi
        done
        sleep 1
            
    done

}

task_delete(){

    for target in ${TARGETS[@]};
        do
            ( echo "startDestructor" ; sleep 2; ) | telnet $target &
        done
    # aspetto che tutti i comandi telnet siano terminati
    wait

}

reboot_rasp(){

    for target in ${TARGETS[@]};
        do
            ( echo "cmd" ; sleep 2 ; echo "reboot -c" ; sleep 2 ) | telnet $target &
        done
    
    rm /tmp/fifo*

}

reboot_devices(){

    #TODO: Controllare se necessario rimuovere le code
    rm /tmp/fifo*
    
    echo "Accensione dei raspberry in corso..."
    python3 ./tapo_control.py "turnOn"
    sleep 35
    echo "Presa di alimentazione accesa"

}

shutdown(){

    #TODO: Controllare se necessario rimuovere le code
    rm /tmp/fifo* 

    python3 ./tapo_control.py "turnOff"
    sleep 1

}

see_log(){
    # Controlla se esiste la cartella dove loggare
    if [ ! -d "./execution_workbench_log_files" ];
    then 
        mkdir ./execution_workbench_log_files
    fi

    # Lancia il file che mette l'host in ascolto
    host_ip=$( grep -Po '(?<=\[Host_ip\] : ")[^"]*' ./build.config )
    python3 ./../host_script/log.py 5 $host_ip $PWD &
    sleep 1

    # Esegui il comando ai nodi per inviare il log verso l'host 
    for target in ${TARGETS[@]};
        do
            ( echo "sendLogToHost"; sleep 1) | telnet $target  & 
        done
    
    wait
}

help_workbench(){

echo -e "Requisiti:
    -rpivsb            Estrarre l'archivio rpivsb.rar nella root del git
    -vxWorks           Copiare il file vxWorks da onedrive nella cartella /connect/
    -build.config      Copiare il file build_example.config e rinominarlo in build.config modificando le voci opportunamente
    -SDK               Copiare la cartella contenente l'sdk del raspberry nella stessa directory contenente il repository
    "
echo -e "Opzioni: 
        -c  (connect)          Si connette ai raspberry e apre le shell telnet per eseguire i comandi in remoto
        -b  (build)            Esegue il Build del progetto, in caso di errori salva lo stato in workbench_log_files/build_log.txt
        -l  (load)             Esegue il Load del modulo sui raspberry
        -u  (unload)           Esegue l'unload del modulo sui raspberry
        -o  (output)           Mostro i log dei raspberry
        -d  (delete)           Effettua il task delete nei raspberry
        -r  (reboot)           Effettua il reboot dei raspberry
        -t  (hard reboot)      Effettua uno spegnimento e accensione dell'alimentazione 
        -s  (shutdown)         Effettua lo spegnimento completo del sistema
        -h  (help)             Mostra questo messaggio
    "
}

while getopts "cbluodrsth" options;
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
                # Messaggio di echo già presente nella funzione load_module
                load_module
                ;;
            u)
                echo "Eseguo l'unload del modulo"
                unload_module
                ;;
            o)
                echo "Mostro i log [parte di output rediretto a /dev/null]"
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
            t)
                echo "Spegnimento e accensione della presa"
                reboot_devices
                ;;
            s)
                echo "Shutdown system"
                shutdown
                ;;
            h)
                help_workbench
                ;;
        esac
    done