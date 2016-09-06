# acquire measurement from oscilloscope
import visa
import sys
import os
import struct
import binascii
import requests,json
from time import sleep 
import subprocess
import pyperclip

subprocess.call([sys.executable, 'set_device_active.py'])

if (len(sys.argv)>1):
   fname = sys.argv[1]  # eg. signal.txt
   subprocess.call([sys.executable, 'load_and_run_csv_signal.py', fname])

rm=visa.ResourceManager()
osc=rm.open_resource('TCPIP::192.168.0.108::inst0::INSTR')
osc.encoding='utf-8' # need this because *IDN? returns AKIP in russian
osc.write_termination=''

nazerbox=rm.open_resource("ASRL8::INSTR", send_end=False)
sleep(3)
nazerbox.baud_rate = 115200
nazerbox.timeout = 2000
nazerbox.read_termination = '\n'

print("FIRING!")
sleep(1)
nazerbox.write('ARM\n')
sleep(2)
q=float(osc.query(':MEASure:DELTatime? CHAN2, CHAN4')) # 
if (q<=0) | (q>1):
   q=-1

pyperclip.copy("%6.6f" % float(q))
print("Delta time=%6.6f s (copied to clipboard)" % float(q))
with open("results.csv", "a") as f:
    f.write("%6.6f\n" % float(q))
