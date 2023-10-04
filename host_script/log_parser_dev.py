import re
import datetime
import sys
import os
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import numpy as np
import matplotlib.transforms as transforms


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

# In questo modo i path sono assoluti (lo script può essere eseguito da qualsiasi directory)
abs_path = os.path.abspath(__file__)
HOST_SCRIPT_DIR = os.path.dirname(abs_path)
BASE_DIR = os.path.dirname(HOST_SCRIPT_DIR)

# Questo numero è superiore al numero di transizioni presenti nella rete
# per evitare che aggiunti future di nuove transizioni vadano a richiedere
# la modifica di questo valore
N_TOTAL_TRANSITIONS = 90

analysis_path = f"{HOST_SCRIPT_DIR}/analysis"
isExist = os.path.exists(analysis_path)
if not isExist:
    # Create a new directory because it does not exist
    os.makedirs(analysis_path)

type1_path = f"{HOST_SCRIPT_DIR}/importer_files"
isExist = os.path.exists(type1_path)
if not isExist:
    # Create a new directory because it does not exist
    os.makedirs(type1_path)

for log_idx in range(1,6):
    # Extract the info from the logs
    log_path = f"{BASE_DIR}/connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
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

    countings = {}
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

            if result in countings:
                countings[result] += 1
            else:
                countings[result] = 1
            
            # Check if it's the first time that we encounter a transition
            if transitions[trans_num][0] == "-":
                # If first time create a list with a single value
                transitions[trans_num] = [result]
            elif isinstance(transitions[trans_num][0], int):
                # If not first time append to the list
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
    max_execution_times = dict(sorted(max_execution_times.items()))        

    # Create max graph
    times = []
    occurrencies = []        
    for time, trans_list in max_execution_times.items():
        times.append(time)
        occurrencies.append(len(trans_list))
    
    n = len(times)
    colors = list(mcolors.TABLEAU_COLORS.values())
    colors = colors[:n] if n <= len(colors) else colors * (n // len(colors)) + colors[:n % len(colors)]
    plt.style.use("ggplot")
    fig, ax = plt.subplots(dpi=288)
    bar_container = ax.bar(times, occurrencies, color=colors, width=2)
    ax.set(ylabel="Occurrencies", title="Max execution times [ms]")
    ax.tick_params(axis='x', labelsize=6)
    ax.set_xticks(times)
    fig.savefig(f'{analysis_path}/transitions_max_192.186.1.21{log_idx}.png')

    # Create execution times graph
    countings = dict(sorted(countings.items()))
    times = list(countings.keys())
    occurrencies = list(countings.values())

    n = len(times)
    colors = list(mcolors.TABLEAU_COLORS.values())
    colors = colors[:n] if n <= len(colors) else colors * (n // len(colors)) + colors[:n % len(colors)]
    plt.style.use("ggplot")
    fig, ax = plt.subplots(dpi=288)
    bar_container = ax.bar(times, occurrencies, color=colors, width=2)

    for bar, occurrency, num in zip(bar_container, occurrencies, range(0, n)):
        ax.text(bar.get_x() + bar.get_width()/2, 10 if num % 2 == 0 else 20, str(occurrency),
                ha='center', va='center', rotation='vertical', fontsize=6)
    ax.set(ylabel="Occurrencies", title="Execution times [ms]")
    ax.tick_params(axis='x', labelsize=4)
    ax.set_xticks(times, labels=times, visible=False)
    tick_labels = times
    for i, tick_label in enumerate(tick_labels):
        if i % 2 == 1:  # Se l'indice è dispari, sposta l'etichetta verso il basso
            trans = transforms.Affine2D().translate(0, -5) + ax.transData
            ax.text(tick_label, 0, str(tick_label), transform=trans, ha='center', fontsize=6)
        else:  # Altrimenti, posiziona l'etichetta normalmente
            trans = transforms.Affine2D().translate(0, -8) + ax.transData
            ax.text(tick_label, 0, str(tick_label), transform=trans, ha='center', fontsize=6)
    ax.set_ylim(0, 100)
    fig.savefig(f'{analysis_path}/transitions_exec_192.186.1.21{log_idx}.png')
    

def draw_task_execution_times():

    task_start_end = {
        "ctrlTask" : ["t81","t76"],
        "wifiTask" : ["t54","t77"],
        "diagTask" : ["t82","t21"],
        "posiTask" : ["t83","t80"],
    }

    task_times = {
        "ctrlTask" : [],
        "wifiTask" : [],
        "diagTask" : [],
        "posiTask" : [],
    }

    for log_idx in range(1,6):
        #resetto così da poter utilizzare lo stesso dict con tutti i log
        for task_name in task_times:
            task_times[task_name].clear()
        log_path = f"{BASE_DIR}/connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
        with open(log_path, "r") as f:
            log = f.read()
            match_list = re.findall("(\d+)    (\w+)    -*\[(t\d+)\]",log)

            for time, task_name, trans_name in match_list:
                # Converto in ms e salvo i tempi delle trans di start come negativi, così sommando quelli di end ottengo il tempo trascorso
                if task_start_end[task_name][0] == trans_name:
                    task_times[task_name].append(- round(int(time) / 1000))
                elif task_start_end[task_name][1] == trans_name:
                    task_times[task_name][-1] += round(int(time) / 1000)
            #rimuovo le transizioni negative se presenti (ultima transizione del log è stata uno start e non è stato catturato l'end)
            for task_name in task_times:
                for time_idx, time in enumerate(task_times[task_name]):
                    if time < 0:
                        task_times[task_name].pop(time_idx)

        if log_idx == 2 :
            fig, axs = plt.subplots(len(task_times), sharex= False, constrained_layout=True)
        else:
            fig, axs = plt.subplots(len(task_times)-1, sharex= False, constrained_layout=True)
            axs[1].sharex(axs[0])
        fig.set_size_inches(10,10)
        fig.suptitle("Distribution of task termination times", fontsize=20 ,fontweight='bold')
        bins_n = 20
        colors = ["red", "blue", "green", "orange"]

        

        for task_idx, task_name in enumerate(task_times):
            if log_idx != 2 and task_name == "posiTask":
                continue
            axs[task_idx].hist(task_times[task_name], bins=bins_n, color=colors[task_idx])
            axs[task_idx].ticklabel_format(useOffset=False)
            axs[task_idx].set_title(task_name, fontsize=14 ,fontweight='bold')
            axs[task_idx].set_xlabel("time (ms)")
        
        plt.savefig(f'{HOST_SCRIPT_DIR}/analysis/task_times_192.186.1.21{log_idx}.png')


# draw_task_execution_times()