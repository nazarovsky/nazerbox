# acquire measurement from oscilloscope
import visa
import sys
import os
import struct
import binascii
from time import sleep 


print('Default Oscilloscope Setup...')
rm=visa.ResourceManager()
osc=rm.open_resource('TCPIP::192.168.0.108::INSTR')
osc.encoding='utf-8' 
osc.timeout = 2000
osc.write_termination=''

# reset
osc.write('*RST')
# set up time characteristics
osc.write(':TIMebase:RANGe 2E0')
osc.write(':TIMebase:DELay 0')
osc.write(':TIMebase:REFerence CENTer')
# set up channels
osc.write(':CHANnel1:PROBe 1.0') # attenuation
osc.write(':CHANnel1:RANGe 20.0') # range  v/ full scale
osc.write(':CHANnel1:OFFSet 0.0') # offset
osc.write(':CHANnel1:INPut DC') # coupling


osc.write(':CHANnel2:PROBe 1.0') # attenuation
osc.write(':CHANnel2:RANGe 20.0') # range  v/ full scale
osc.write(':CHANnel2:OFFSet 0.0') # offset
osc.write(':CHANnel2:INPut DC') # coupling


osc.write(':CHANnel3:PROBe 1.0') # attenuation
osc.write(':CHANnel3:RANGe 20.0') # range  v/ full scale
osc.write(':CHANnel3:OFFSet 0.0') # offset
osc.write(':CHANnel3:INPut DC') # coupling


osc.write(':CHANnel4:PROBe 1.0') # attenuation
osc.write(':CHANnel4:RANGe 20.0') # range  v/ full scale
osc.write(':CHANnel4:OFFSet 0.0') # offset
osc.write(':CHANnel4:INPut DC') # coupling

osc.write(':CHANnel1:DISPlay ON') # Activate
osc.write(':CHANnel2:DISPlay ON') # 
osc.write(':CHANnel3:DISPlay ON') # 
osc.write(':CHANnel4:DISPlay ON') # 

osc.write(':SYSTem:HEADer OFF') # 

# set up trigger
osc.write(':TRIGger:SWEep TRIGgered') #  Normal
osc.write(':TRIGger:LEVel CHAN2,2.5') # level 2.5 v
osc.write(':TRIGger:MODE EDGE') # 
#osc.write(':TRIGger:COMM:SOURce CHAN3') # 
osc.write(':TRIGger:EDGE:SOURce CHAN2') # 
osc.write(':TRIGger:EDGE:SLOPe POSitive') # 

osc.write(':MEASure:DELTatime:DEFine RISing,1,MIDDle,FALLing,1,MIDDle') # 
print('Done');