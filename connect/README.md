Requisiti:

    -Copiare il file build_example.config e rinominarlo in build.config modificando le voci opportunamente

    -Estrarre l'archivio rpivsb.rar nella root del git

    -copiare il file vxWorks (kernel per connettersi al raspberry) da onedrive nella cartella connect

Opzioni dello script workbench:

        -c  (compile)          Si connette ai raspberry e apre le shell telnet per eseguire i comandi in remoto
        -b  (build)            Esegue il Build del progetto, in caso di errori salva lo stato in log_files/build_log.txt
        -l  (load)             Esegue il Load del modulo sui raspberry
        -h  (help)             Mostra questo messaggio

    
