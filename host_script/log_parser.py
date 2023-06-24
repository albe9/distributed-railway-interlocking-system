import re
import datetime
import sys


TASK_DEADLINES = {
    "ctrlTask" : 100,
    "wifiTask" : 100,
    "diagTask" : 100,
    "positioningTask" : 100
}


for log_idx in range(1,6):
    log_path = "./log_test/log_192.168.1.21" + str(log_idx) + ".txt"
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
    
    type2_path = "./importer_files/type2_192.168.1.21" + str(log_idx) + ".txt"
    with open(type2_path, "w") as f:

        task_transictions_dict = {}
        for line in extract:
            line=line.strip()
            match = re.search("ms (.*Task)    -*\[(t\d*)\]", line)
            if match != None:
                task_name = match.group(1)
                transition_name = match.group(2)
                if task_name not in task_transictions_dict:
                    task_transictions_dict[task_name] = []
                task_transictions_dict[task_name].append(transition_name)
        
        f.write(f"PROCESSES: {len(task_transictions_dict)}\n")
        for task_number,task_name in enumerate(task_transictions_dict):
            transitions = task_transictions_dict[task_name]
            f.write(f"process{task_number + 1}TransitionsNumber: {len(transitions)}\n")
            for transition in transitions:
                f.write(f"{transition}\n")
        # print(task_transictions_dict)
    