## Requisiti:

- Nella cartella connect copiare il file build_example.config e rinominarlo in build.config modificando le voci opportunamente.

- Estrarre l'archivio rpivsb.rar nella root del git.

- copiare il file vxWorks (kernel per connettersi al raspberry) da onedrive nella cartella connect.

<br>

## Opzioni dello script workbench:

<br>

        
| Option | Description |
| ------ | ------ |
| -c  (compile) | Si connette ai raspberry e apre le shell telnet per eseguire i comandi in remoto |
| -b  (build)   | Esegue il Build del progetto, in caso di errori salva lo stato in log_files/build_log.txt |
| -l  (load)    | Esegue il Load del modulo sui raspberry |
| -o  (output)  | Mostro i log dei raspberry |
| -d  (delete)  | Effettua il task delete nei raspberry (per ora task_wifi) |
| -r  (reboot)  | Effettua il reboot dei raspberry |
| -h  (help)    | Mostra questo messaggio |

    
