host_ip=$( grep -Po '(?<=\[Host_ip\] : ")[^"]*' ./build.config )
python3 ./../host_script/host.py 3 $host_ip