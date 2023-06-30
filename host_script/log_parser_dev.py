import re
import datetime
import sys
import os, glob

debug = True

"""
Creare un documento di tipo "File Importer Box" e importare nel progetto
due file di testo:
1) il log (RTAI_relative_millisec_1.txt)
2) la definizione del task-set (task_set_def.txt).

Circa il formato dei file di tipo 1:
t3 -> prima transizione che esegue
40 -> tempo relativo di firing della prima transizione (misurato dal
firing precedente)
t0 -> seconda transizione che esegue
0  -> tempo relativo di firing della seconda transizione (misurato dal
firing precedente)
t4 -> terza transizione che esegue
1  -> tempo relativo di firing della terza transizione (misurato dal
firing precedente)
...

Circa il formato dei file di tipo 2:
PROCESSES: 5 -> numero di task
process1TransitionsNumber: 3 -> numero di transizioni del primo task
t0 -> prima transizione del primo task
t1 -> seconda transizione del primo task
t2 -> terza transizione del primo task
deadline: 40 ->  -> deadline del primo task
process2TransitionsNumber: 3
t3
t4
t5
deadline: 40
process3TransitionsNumber: 6
t6
t7
t8
t9
t10
t11
deadline: 80
process4TransitionsNumber: 8
t12
t13
t14
t15
t16
t17
t18
t19
deadline: 100
process5TransitionsNumber: 5
t20
t21
t22
t23
t24
deadline: 120
CONTROLLABLE_TRANSITIONS: 0 -> lasciare zero
ALIAS_IDENTIFIERS: 0 -> lasciare zero

Quando si crea un documento del plugin "PetriSimulator", si devono
fornire in ingresso i seguenti file:
- modello PTPN (PTPN/PTPN.pef.xml)
- grafo delle classi di stato in formato xml (TCAnalyzer/output.xml)
- grafo delle classi di stato in formato txt (TCAnalyzer/output.txt)
- grafo delle classi di stato in formato graphml (TCAnalyzer/output.graphml)
- definizione del task-set (File Importer Box/task_set_def.txt)
- log (File Importer Box/RTAI_relative_millisec_1.txt)

Una volta completata la simulazione del log, sono prodotti vari file. In
particolare:
- ProfilingMeasure.txt riporta gli istogrammi dei tempi di esecuzione di
ciascuna transizione
- ReleaseCompletionAnalysis.txt indica se sono state mancate deadline
- CoverageMeasure.txt riporta il numero di nodi e archi coperti nel
grafo delle marcature e in quello delle classi
"""

# TASK_DEADLINES = {
#     "ctrlTask" : 100,
#     "wifiTask" : 100,
#     "diagTask" : 100,
#     "positioningTask" : 100
# }


# for log_idx in range(1,6):
#     log_path = "./../connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
#     with open(log_path, "r") as f:
#         extract = f.readlines()
#     type1_path = "./importer_files/type1_192.168.1.21" + str(log_idx) + ".txt"
#     with open(type1_path, "w") as f:
#         prev_time = None
#         for line in extract:
#             line=line.strip()

#             match = re.search("(\d*h:\d*m:\d*s:\d*ms).*\[(t\d*)\]",line)
            
#             if match is not None:
#                 time_str = match.group(1)
#                 current_time = datetime.datetime.strptime(time_str, "%Hh:%Mm:%Ss:%fms").time()
#                 current_time_td = datetime.timedelta(hours=current_time.hour,
#                                                     minutes=current_time.minute,
#                                                     seconds=current_time.second,
#                                                     milliseconds=current_time.microsecond/1000)
#                 if prev_time is None:
#                     prev_time = current_time_td
                
#                 result = current_time_td - prev_time
#                 prev_time = current_time_td

#                 result_ms = int(result.total_seconds() * 1000)

#                 thread_name = match.group(2)
#                 f.write(f"{thread_name}\n{result_ms}\n")
    
#     type2_path = "./importer_files/type2_192.168.1.21" + str(log_idx) + ".txt"
#     with open(type2_path, "w") as f:

#         task_transictions_dict = {}
#         for line in extract:
#             line=line.strip()
#             match = re.search("ms (.*Task)    -*\[(t\d*)\]", line)
#             if match != None:
#                 task_name = match.group(1)
#                 transition_name = match.group(2)
#                 if task_name not in task_transictions_dict:
#                     task_transictions_dict[task_name] = []
#                 task_transictions_dict[task_name].append(transition_name)
        
#         f.write(f"PROCESSES: {len(task_transictions_dict)}\n")
#         for task_number,task_name in enumerate(task_transictions_dict):
#             transitions = task_transictions_dict[task_name]
#             f.write(f"process{task_number + 1}TransitionsNumber: {len(transitions)}\n")
#             for transition in transitions:
#                 f.write(f"{transition}\n")
#             f.write(f"deadline: {TASK_DEADLINES[task_name]}\n")
#         f.write("CONTROLLABLE_TRANSITIONS: 0\n")
#         f.write("ALIAS_IDENTIFIERS: 0\n")
#         # print(task_transictions_dict)

# Pulizia della cartella in cui si andranno a salvare le tracce
trace_files= "trace_files/*"
files = glob.glob(trace_files)
for f in files:
    os.remove(f)

first_last_transitions = [{"first":"t18", "last": ("t10", "t38"),"max_runs_to_get": (100, 5)},
                          {"first":"t27", "last": ("t28", "t3"), "max_runs_to_get": (100, 100)},
                          {"first":"t17", "last": ("t20", "t21"), "max_runs_to_get": (100, 100)}]

# first_last_transitions = [{"first":"t18", "last": ("t10", "t38")},
#                             {"first":"t27", "last": ("t28", "t3")},
#                             {"first":"t17", "last": ("t20", "t21")}]


for f_l_tr in first_last_transitions:
    # Ricerca su tutte le tracce d'interesse

    for log_idx in range(1,2):
        # Esegui la ricerca su ogni rasp
        prev_time = None
        found_start = False
        found_end = False
        trace_started = False
        trace_log = ""
        # Creiamo un array di lunghezza pari al numero di possibili transizioni di end in cui segneremo quante volte una traccia
        # si è conclusa con quella particolare transizione
        trace_found_num = [0] * len(f_l_tr["max_runs_to_get"])
        

        log_path = "./../connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
        with open(log_path, "r") as f:
            extract = f.readlines()

        for line in extract:
            line = line.strip()

            match = re.search("(\d*h:\d*m:\d*s:\d*ms) (.*Task).*\[(t\d*)\]",line)
            
            if match is not None:
                # Calcolo tempo di esecuzione della transizione letta rispetto alla transizione precedente
                time_str = match.group(1)
                current_time = datetime.datetime.strptime(time_str, "%Hh:%Mm:%Ss:%fms").time()
                current_time_td = datetime.timedelta(hours=current_time.hour,
                                                    minutes=current_time.minute,
                                                    seconds=current_time.second,
                                                    milliseconds=current_time.microsecond/1000)
                if prev_time is None:
                    prev_time = current_time_td
                
                result = current_time_td - prev_time
                prev_time = current_time_td

                result_ms = int(result.total_seconds() * 1000)

                # Si decide se la transizione fa parte della run d'interesse
                transition_id = match.group(3)
                if (transition_id == f_l_tr["first"] and not trace_started):
                    # Entriamo qui se la transizione è di start e ancora non avevamo trovato un inizio di traccia
                    found_start = True
                    trace_started = True
                    start_transition = transition_id
                    # Nel caso di debug attivo aggiungiamo al file quale è la riga che ha avviato la traccia
                    if debug:
                        trace_log += line + "\n"
                elif (transition_id in f_l_tr["last"]):
                    # Entriamo qui se la transizione è di end
                    found_end = True
                    stop_transition = transition_id
                else:
                    # Entriamo qui se la transizione trovata non è di interesse oppure è tra start e end
                    pass

                if found_start:
                    # Se in passato [o alla riga corrente] è stata trovata la transizione di start, allora la transizione attuale è 
                    # tra start ed end [oppure start ed end stesse], quindi deve essere aggiunta
                    trace_log += f"{transition_id}\n{result_ms}\n"       

                if found_end:
                    # Se alla riga corrente è stata trovata una transizione di end vuol dire che la traccia è stata completata,
                    # quindi settiamo che non abbiamo ancora trovato inizio e fine della nuova traccia e che la nuova traccia non è
                    # ancora iniziata  
                    found_start = False
                    found_end = False
                    trace_started = False
                    
                    # Vediamo quale è l'indice (tra le transizioni possibili) della transizione che ha fatto concludere la traccia
                    tr_idx = f_l_tr["last"].index(transition_id)                    

                    if trace_found_num[tr_idx] <= f_l_tr["max_runs_to_get"][tr_idx]:
                        # Se non abbiamo ancora estratto dal log il numero di run di quella traccia che ci basta salviamo la
                        # traccia di esecuzione
                        trace_path = f"./trace_files/192.168.1.21{log_idx}_{start_transition}_{stop_transition}_n{trace_found_num[tr_idx]}.txt"
                        with open(trace_path, "a") as f:       
                            f.write(trace_log)

                    # Aumentiamo il contatore di questa particolare tipologia di traccia trovata e azzeriamo il log della traccia                   
                    trace_found_num[tr_idx] += 1
                    trace_log = ""