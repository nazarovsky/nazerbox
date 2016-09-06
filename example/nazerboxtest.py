import visa
import sys
from time import sleep 

rm=visa.ResourceManager()
nazerbox=rm.open_resource("ASRL8::INSTR", send_end=False)
sleep(3)
nazerbox.baud_rate = 115200
nazerbox.timeout = 2000
nazerbox.read_termination = '\n'

q = nazerbox.query('*IDN?\n') # send indentification command and print response
print(q)
nazerbox.query('WIDTH 500\n') # set up impulse width to 500 ms
sleep(1)
nazerbox.query('DELAY 100\n') # set up delay from reference input signal front to 100 ms
sleep(1)
nazerbox.query('NUM 5\n')  # set up number of generated impulses to 5
sleep(1)
nazerbox.write('ARM\n')  # fire on next impulse on reference input signal front
sleep(20)
