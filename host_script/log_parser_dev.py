import re
import datetime
import sys
import os
import matplotlib.pyplot as plt
import matplotlib.ticker 
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


def draw_transition_times():

    for log_idx in range(1,6):
        # Extract the info from the logs
        log_path = f"{BASE_DIR}/connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
        with open(log_path, "r") as f:
            extract = f.readlines()

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
        fig, ax = plt.subplots(1, 2, sharey=True)
        fig.set_size_inches(14,10)
        fig.subplots_adjust(wspace=0.05)

        x_time_sep = 40
        times_occurrencies_left = [[t[0] for t in zip(times, occurrencies) if t[0] <= x_time_sep],
                                   [t[1] for t in zip(times, occurrencies) if t[0] <= x_time_sep]]
        times_occurrencies_right = [[t[0] for t in zip(times, occurrencies) if t[0] > x_time_sep],
                                   [t[1] for t in zip(times, occurrencies) if t[0] > x_time_sep]]
        left_colors = colors[:len(times_occurrencies_left[0])]
        right_colors = colors[len(times_occurrencies_left[0]):]

        bar_container = []
        bar_container.append(ax[0].bar(times_occurrencies_left[0],
                                       times_occurrencies_left[1],
                                        color=left_colors, width=0.5, log=True))
        bar_container.append(ax[1].bar(times_occurrencies_right[0],
                                       times_occurrencies_right[1],
                                        color=right_colors, width=1.5, log=True))
        
        for bar, occurrency, num in zip(bar_container[0], times_occurrencies_left[1], range(0, n)):
            ax[0].text(bar.get_x() + bar.get_width()/10, np.sqrt(bar.get_height()) if bar.get_height() != 1 else 0.9, str(occurrency),
                    ha='left', va='center', rotation='vertical', fontsize=6)    
        
        for bar, occurrency, num in zip(bar_container[1], times_occurrencies_right[1], range(0, n)):
            ax[1].text(bar.get_x() + bar.get_width()/10, np.sqrt(bar.get_height()) if bar.get_height() != 1 else 0.9, str(occurrency),
                    ha='left', va='center', rotation='vertical', fontsize=6)


        fig.suptitle("Max execution times [ms]", fontsize=20 ,fontweight='bold')
        ax[0].annotate('ms', xy=(0.98, -0.07), xycoords='axes fraction',
             horizontalalignment='right', verticalalignment='center', fontsize=12)
        
        ax[0].annotate("Occurrencies", xy=(-0.1, 0.5), xycoords='axes fraction',
             horizontalalignment='right', verticalalignment='center', fontsize=12, rotation=90)
        
        ax[0].tick_params(axis='x', labelsize=6)
        ax[0].set_xticks(times_occurrencies_left[0])
        ax[1].tick_params(axis='x', labelsize=6)
        ax[1].set_xticks(times_occurrencies_right[0])

        ax[0].set_xlim(-1, x_time_sep + 1)
        ax[1].set_xlim(x_time_sep, max(times_occurrencies_right[0]) + 5)

        # hide the spines between ax and ax2
        ax[0].spines.right.set_visible(False)
        ax[1].spines.left.set_visible(False)
        ax[1].tick_params(which="both", left=False)

        d = .5  # proportion of vertical to horizontal extent of the slanted line
        kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
                    linestyle="none", color='k', mec='k', mew=1, clip_on=False)
        ax[0].plot([1, 1], [0, 1], transform=ax[0].transAxes, **kwargs)
        ax[1].plot([0, 0], [0, 1], transform=ax[1].transAxes, **kwargs)

        fig.savefig(f'{analysis_path}/transitions_max_192.186.1.21{log_idx}.png', dpi=300)



        # Create execution times graph
        countings = dict(sorted(countings.items()))
        times = list(countings.keys())
        occurrencies = list(countings.values())

        n = len(times)
        colors = list(mcolors.TABLEAU_COLORS.values())
        colors = colors[:n] if n <= len(colors) else colors * (n // len(colors)) + colors[:n % len(colors)]

        plt.style.use("ggplot")
        fig, ax = plt.subplots(1, 2, sharey=True)
        fig.set_size_inches(14,10)
        fig.subplots_adjust(wspace=0.05)

        x_time_sep = 30
        times_occurrencies_left = [[t[0] for t in zip(times, occurrencies) if t[0] <= x_time_sep],
                                   [t[1] for t in zip(times, occurrencies) if t[0] <= x_time_sep]]
        times_occurrencies_right = [[t[0] for t in zip(times, occurrencies) if t[0] > x_time_sep],
                                   [t[1] for t in zip(times, occurrencies) if t[0] > x_time_sep]]
        left_colors = colors[:len(times_occurrencies_left[0])]
        right_colors = colors[len(times_occurrencies_left[0]):]

    
        bar_container = []
        bar_container.append(ax[0].bar(times_occurrencies_left[0],
                                       times_occurrencies_left[1],
                                        color=left_colors, width=0.5, log=True))
        bar_container.append(ax[1].bar(times_occurrencies_right[0],
                                       times_occurrencies_right[1],
                                        color=right_colors, width=1.5, log=True))

        for bar, occurrency, num in zip(bar_container[0], times_occurrencies_left[1], range(0, n)):
            ax[0].text(bar.get_x() + bar.get_width()/10, np.sqrt(bar.get_height()/2), str(occurrency),
                    ha='left', va='center', rotation='vertical', fontsize=6)    
        
        for bar, occurrency, num in zip(bar_container[1], times_occurrencies_right[1], range(0, n)):
            ax[1].text(bar.get_x() + bar.get_width()/10, np.sqrt(bar.get_height()/2), str(occurrency),
                    ha='left', va='center', rotation='vertical', fontsize=6)

        fig.suptitle("Execution times [ms]", fontsize=20 ,fontweight='bold')
        ax[0].annotate('ms', xy=(0.98, -0.07), xycoords='axes fraction',
             horizontalalignment='right', verticalalignment='center', fontsize=12)
        
        ax[0].annotate("Occurrencies", xy=(-0.1, 0.5), xycoords='axes fraction',
             horizontalalignment='right', verticalalignment='center', fontsize=12, rotation=90)


        ax[0].set_xlim(-1, x_time_sep + 1)
        ax[1].set_xlim(x_time_sep, max(times_occurrencies_right[0]) + 5)

        # hide the spines between ax and ax2
        ax[0].spines.right.set_visible(False)
        ax[1].spines.left.set_visible(False)
        ax[1].tick_params(which="both", left=False)

        d = .5  # proportion of vertical to horizontal extent of the slanted line
        kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
                    linestyle="none", color='k', mec='k', mew=1, clip_on=False)
        ax[0].plot([1, 1], [0, 1], transform=ax[0].transAxes, **kwargs)
        ax[1].plot([0, 0], [0, 1], transform=ax[1].transAxes, **kwargs)

        fig.savefig(f'{analysis_path}/transitions_exec_192.186.1.21{log_idx}.png', dpi=300)


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

        task_distribution_times = {"ctrlTask" : [[],[]],
                                   "wifiTask" : [[],[]],
                                   "diagTask" : [[],[]],
                                   "posiTask" : [[],[]],
                                   }
        
        for task_name in task_times:
            temp_dict = {}
            for times in task_times[task_name]:
                if times not in temp_dict:
                    temp_dict[times] = 1
                else :
                    temp_dict[times] += 1
            for different_times in temp_dict:
                task_distribution_times[task_name][0].append(different_times)
                task_distribution_times[task_name][1].append(temp_dict[different_times])


        fig, axs = plt.subplots(1, 2, sharey= True)
        fig.set_size_inches(10,10)
        fig.subplots_adjust(wspace=0.05)
        fig.suptitle("Distribution of task termination times", fontsize=20 ,fontweight='bold')
        colors = ["red", "blue", "green", "orange"]
        alpha = [1, 0.6, 0.6, 0.6]
        
        # hide the spines between ax and ax2
        axs[0].spines.right.set_visible(False)
        axs[1].spines.left.set_visible(False)
        axs[1].tick_params(which="both", left=False)


        separator_value = 200

        for task_idx, task_name in enumerate(task_times):
            if log_idx != 2 and task_name == "posiTask":
                continue
            axs[0].bar(task_distribution_times[task_name][0], 
                       task_distribution_times[task_name][1], width=2, color=colors[task_idx], alpha=alpha[task_idx], label=task_name, log=True)
            axs[1].bar(task_distribution_times[task_name][0], 
                       task_distribution_times[task_name][1], width=15, color=colors[task_idx], alpha=alpha[task_idx], label=task_name, log=True)
            axs[0].set_xlim(0, separator_value)
            axs[1].set_xlim(1900, 3100)

        d = .5  # proportion of vertical to horizontal extent of the slanted line
        kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
                    linestyle="none", color='k', mec='k', mew=1, clip_on=False)
        axs[0].plot([1, 1], [0, 1], transform=axs[0].transAxes, **kwargs)
        axs[1].plot([0, 0], [0, 1], transform=axs[1].transAxes, **kwargs)

        axs[0].annotate('ms', xy=(0.98, -0.07), xycoords='axes fraction',
             horizontalalignment='right', verticalalignment='center', fontsize=12)
        axs[0].annotate("Occurrencies", xy=(-0.1, 0.5), xycoords='axes fraction',
             horizontalalignment='right', verticalalignment='center', fontsize=12, rotation=90)

        plt.legend()
        plt.savefig(f'{HOST_SCRIPT_DIR}/analysis/task_times_192.186.1.21{log_idx}.png', dpi=300)


        

draw_transition_times()
draw_task_execution_times()