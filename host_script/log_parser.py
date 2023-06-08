import re
import datetime
import sys

for log_idx in range(1,6):
    log_path = "./log_test/log_192.168.1.21" + str(log_idx) + ".txt"
    with open(log_path, "r") as f:
        extract = f.readlines()
    new_log_path = "./log_test/new_log_192.168.1.21" + str(log_idx) + ".txt"
    with open(new_log_path, "w") as f:
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