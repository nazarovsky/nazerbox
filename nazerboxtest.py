import visa
import sys
from time import sleep 

#   Supported SCPI commands are:
#   *  *IDN?         -> identify
#   *  ARM           -> ARM device to fire on next PPS
#   *  DELAY (?)     -> GET / SET delay (from 0 to 1000 ms)
#   *  WIDTH (?)     -> GET / SET impulse width (from 1 to 1000 ms)
#   *  NUM (?)       -> GET / SET impulse count (from 1 to 100)

rm=visa.ResourceManager()
nazerbox=rm.open_resource("ASRL16::INSTR", send_end=False)
sleep(3)
nazerbox.baud_rate = 115200
nazerbox.timeout = 2000
nazerbox.read_termination = '\n'

q = nazerbox.query('*IDN?\n')
print(q)
nazerbox.query('WIDTH 500\n')
sleep(1)
nazerbox.query('DELAY 100\n')
sleep(1)
nazerbox.query('NUM 5\n')
sleep(1)
nazerbox.write('ARM\n')
sleep(20)
