import re
import datetime
import sys

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

TASK_DEADLINES = {
    "ctrlTask" : 100,
    "wifiTask" : 100,
    "diagTask" : 100,
    "positioningTask" : 100
}


for log_idx in range(1,6):
    log_path = "./../connect/execution_log_files/log_192.168.1.21" + str(log_idx) + ".txt"
    with open(log_path, "r") as f:
        extract = f.readlines()
    type1_path = "./importer_files/type1_192.168.1.21" + str(log_idx) + ".txt"
    with open(type1_path, "w") as f:
        prev_time = None
        for line in extract:
            line=line.strip()

            match = re.search("(\d*h:\d*m:\d*s:\d*ms).*\[(t\d*)\]",line)
            
            if match is not None:
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

                thread_name = match.group(2)
                f.write(f"{thread_name}\n{result_ms}\n")
    
    # type2_path = "./importer_files/type2_192.168.1.21" + str(log_idx) + ".txt"
    # with open(type2_path, "w") as f:

    #     task_transictions_dict = {}
    #     for line in extract:
    #         line=line.strip()
    #         match = re.search("ms (.*Task)    -*\[(t\d*)\]", line)
    #         if match != None:
    #             task_name = match.group(1)
    #             transition_name = match.group(2)
    #             if task_name not in task_transictions_dict:
    #                 task_transictions_dict[task_name] = []
    #             task_transictions_dict[task_name].append(transition_name)
        
    #     f.write(f"PROCESSES: {len(task_transictions_dict)}\n")
    #     for task_number,task_name in enumerate(task_transictions_dict):
    #         transitions = task_transictions_dict[task_name]
    #         f.write(f"process{task_number + 1}TransitionsNumber: {len(transitions)}\n")
    #         for transition in transitions:
    #             f.write(f"{transition}\n")
    #         f.write(f"deadline: {TASK_DEADLINES[task_name]}\n")
    #     f.write("CONTROLLABLE_TRANSITIONS: 0\n")
    #     f.write("ALIAS_IDENTIFIERS: 0\n")
        # print(task_transictions_dict)
    