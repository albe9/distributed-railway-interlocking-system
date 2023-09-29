import re
import datetime
import sys
import os
import matplotlib.pyplot as plt
import numpy as np


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

N_TOTAL_TRANSITIONS = 80

analysis_path = "./host_script/analysis"
isExist = os.path.exists(analysis_path)
if not isExist:
    # Create a new directory because it does not exist
    os.makedirs(analysis_path)

type1_path = "./host_script/importer_files"
isExist = os.path.exists(type1_path)
if not isExist:
    # Create a new directory because it does not exist
    os.makedirs(type1_path)

for log_idx in range(1,6):
    # Extract the info from the logs
    log_path = "./connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
    with open(log_path, "r") as f:
        extract = f.readlines()

    # Create the type_1 files
    with open(f"{type1_path}/type1_192.168.1.21" + str(log_idx) + ".txt", "w") as f:
        prev_time = None
        for line in extract:
            line=line.strip()

            # match = re.search("(\d*h:\d*m:\d*s:\d*ms).*\[(t\d*)\]",line)
            match = re.search("(\d+)    .*\[(t\d+)\]",line)
            
            if match is not None:
                current_time_micro = int(match.group(1))
                if prev_time is None:
                    prev_time = current_time_micro
                
                result = current_time_micro - prev_time
                prev_time = current_time_micro


                result = round(result / 1000)
                trans_name = match.group(2)
                f.write(f"{trans_name}\n{result}\n")

    # Create a list where element 0 has all the time of executions of transition t0 
    # for example the list will be [[0,0,0], [16,17,16,0], ...]
    transitions = [["-"] for _ in range(N_TOTAL_TRANSITIONS + 1)]
    prev_time = None
    for line in extract:
        line=line.strip()

        # match = re.search("(\d*h:\d*m:\d*s:\d*ms).*\[(t\d*)\]",line)
        match = re.search("(\d+)    .*\[t(\d+)\]",line)
        
        if match is not None:
            current_time_micro = int(match.group(1))
            if prev_time is None:
                prev_time = current_time_micro
            
            result = current_time_micro - prev_time
            prev_time = current_time_micro


            result = round(result / 1000)
            trans_num = int(match.group(2))
            
            # Check if it's the first time that we encounter a transition
            if transitions[trans_num][0] == "-":
                transitions[trans_num] = [result]
            elif isinstance(transitions[trans_num][0], int):
                transitions[trans_num].append(result)

    # Compute statistics
    max_execution_times = {}        
    for idx, transition in enumerate(transitions):
        # Check if transition has data
        if transition[0] != "-":
            max_time = max(transition)
            min_time = min(transition)
            avg_time = sum(transition) / len(transition)
            # print(f"t{idx:02d} --->     Max: {max_time:04d}     Min:{min_time:04d}     Avg:{avg_time:07.2f}")
            if max_execution_times.get(max_time) == None:
                max_execution_times[max_time] = []
            max_execution_times[max_time].append(f"t{idx}")

    # Create graph
    x = []
    y = []        
    for time, trans_list in max_execution_times.items():
        x.append(time)
        y.append(len(trans_list))
    colors = np.random.rand(len(x), 3)
    plt.bar(x, y, color=colors, width=1.6)
    plt.xlabel('Max execution time [ms]')
    plt.ylabel('Occurrences')

    plt.savefig(f'{analysis_path}/transitions_192.186.1.21{log_idx}.png')
        


    

def draw_task_execution_times():

    task_start_end = {
        "ctrlTask" : ["t81","t76"],
        "diagTask" : ["t82","t21"],
        "posiTask" : ["t83","t80"],
        "wifiTask" : ["t54","t77"],
    }

    task_times = {
        "ctrlTask" : [],
        "diagTask" : [],
        "posiTask" : [],
        "wifiTask" : [],
    }

    for log_idx in range(1,6):
        log_path = "./../connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
        with open(log_path, "r") as f:
            log = f.read()
            match_list = re.findall("(wifiTask)",log)
            for match in match_list:
                print(match)
        break
draw_task_execution_times()