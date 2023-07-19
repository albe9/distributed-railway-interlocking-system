# REQUIREMENT: 
#   - pip install PyP100

from PyP100 import PyP100
import sys
import time
import os
import configparser

def restart_device(p100):
    deviceInfo = p100.getDeviceInfo()
    if deviceInfo['result']['device_on']:
        p100.turnOff()
        time.sleep(2)
        p100.turnOn()
    elif not deviceInfo['result']['device_on']:
        p100.turnOn()
    else :
        print("Stato non riconosciuto")

def turnOff_device(p100):
    p100.turnOff()

def main():
    command = sys.argv[1]

    config = configparser.ConfigParser()
    if os.path.isdir("./tapo.config"):
        config.read("./tapo.config")
    else:
        raise FileNotFoundError("File tapo.config non presente")
    user_id = config.get('USER', 'user_id')
    password = config.get('USER', 'password')

    p100 = PyP100.P100("192.168.1.200", user_id, password) 
    p100.handshake()
    p100.login()

    if command == "turnOn":
        restart_device(p100)
    elif command == "turnOff":
        turnOff_device(p100)
    else:
        print("Comando non trovato")

if __name__ == "__main__":
    main()