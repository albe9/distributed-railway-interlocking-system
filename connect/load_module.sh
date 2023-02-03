#!/bin/sh

cd ./../../wrsdk-vxworks7-raspberrypi4b/tools/debug/22.03/x86_64-linux2/bin

gnome-terminal --tab --command "./wrdbg --command  ./../../../../../../distributed-railway-interlocking-system/connect/wrdbg_scripts/load_module1.txt"
gnome-terminal --tab --command "./wrdbg --command  ./../../../../../../distributed-railway-interlocking-system/connect/wrdbg_scripts/load_module2.txt"
gnome-terminal --tab --command "./wrdbg --command  ./../../../../../../distributed-railway-interlocking-system/connect/wrdbg_scripts/load_module3.txt"
gnome-terminal --tab --command "./wrdbg --command  ./../../../../../../distributed-railway-interlocking-system/connect/wrdbg_scripts/load_module4.txt"
gnome-terminal --tab --command "./wrdbg --command  ./../../../../../../distributed-railway-interlocking-system/connect/wrdbg_scripts/load_module5.txt"