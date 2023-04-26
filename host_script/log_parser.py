import sys
import re
import time

# file_path = f"./{sys.argv[1]}.txt"
for log_idx in range(1,6):
    log_path = "./log_test/log_192.168.1.21" + str(log_idx) + ".txt"
    with open(log_path, "r") as f:
        extract = f.readlines()
    new_log_path = "./log_test/new_log_192.168.1.21" + str(log_idx) + ".txt"
    with open(new_log_path, "w") as f:
        for line in extract:
           line=line.strip()
           
           match = re.search("(\d*h:\d*m:\d*s:\d*ms).*\[(t\d*)\]",line)
           if match != None:
               print(match[1], match[2])
               num_transition = match[2]
               ms = int(re.search(":(\d*)ms",match[1])[1])
               ms *= 1000
               ms = f"{ms}ms"
               date_time = re.sub(":(\d*)ms", ms, match[1])
               print(date_time, num_transition)
            # print(time.strptime(match[1]),"%hh:")
            
            
            # cleaned = re.sub(">.*schema#", " ", cleaned)
            