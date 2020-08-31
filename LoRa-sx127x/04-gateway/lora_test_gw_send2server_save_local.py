#!/usr/bin/env python3
# -*- coding: utf8 -*-
""" A simple continuous receiver class. """
# Copyright 2015 Mayer Analytics Ltd.
#
# This file is part of pySX127x.
#
# pySX127x is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
# License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# pySX127x is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You can be released from the requirements of the license by obtaining a commercial license. Such a license is
# mandatory as soon as you develop commercial activities involving pySX127x without disclosing the source code of your
# own applications, or shipping pySX127x with a closed source product.
#
# You should have received a copy of the GNU General Public License along with pySX127.  If not, see
# <http://www.gnu.org/licenses/>.


from time import sleep
import time
import json
import packer
import sys 
import numpy as np
sys.path.insert(0, '../')
from SX127x.LoRa import *
from SX127x.board_config import BOARD
from SX127x.LoRaArgumentParser import LoRaArgumentParser
from datetime import datetime

# in python 2
import urlparse
# in python 3
# import urllib.parse
import requests

import csv

url="http://icmems.ml:3000"

method='/api/sendDatas/'

BOARD.setup()

parser = LoRaArgumentParser("Continous LoRa receiver.")
'''
# in python3
try:
    with open('/home/pi/Documents/test.csv', 'wt', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['name', 'height', 'weight'])
# in python2
except TypeError:
    with open('test.csv', 'wb') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['name', 'height', 'weight'])
'''
# python2
try:
    import sys
    reload(sys)
    sys.setdefaultencoding('utf-8')
except:
    pass

# Create Node list
Nodes = 12        # number of Nodes
NodesFollow = 6  # number of Nodes follow the plant bed
node_list = []
for n in range(Nodes):
    node_list.append("Node"+str(n+1))
for n in range(NodesFollow):
    node_list.append("Node9"+str(n+1))
'''
for n in range(Nodes):
    node_list.append(str(n+1))
for n in range(NodesFollow):
    node_list.append("9"+str(n+1))
'''
number_node = 0            # No. node

class LoRaGateWay(LoRa):
    def __init__(self, verbose=False):
        super(LoRaGateWay, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0,0,0,0,0,0])    # RX
        self._id = "GW_01"


    def on_rx_done(self):
        # print("\nRxDone")
        print('----------------------------------')

        payload = self.read_payload(nocheck=True)
        data = ''.join([chr(c) for c in payload])
        

        try:
            # _length, _data = packer.Unpack_Str(data)
            # print("Time: {}".format( str(time.ctime() )))
            # print("Length: {}".format( len(data) ))
            # print("Raw RX: {}".format( data ))
            info = data.split(",")
            # print(data)
            for i in node_list:
                if(len(info)==5):
                    if(info[0] == i):
                        print("Time: {}".format( str(time.ctime() )))
                        print("This is {}.".format(info[0]))
                        print("temperature:{}".format(float(info[3])))
                        print("humidity:{}".format(float(info[2])))
                        print("par:{}".format(float(info[4])))
                        print("count:{}".format(int(info[1])))
                        
                        # save in Pi...
                        with open('sensorNode' + i[4:] + '.csv', 'a+') as csvfile:
                            writer = csv.writer(csvfile)
                            nowT = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                            writer.writerow([nowT, info[2], info[3], info[4], info[1]]) 

                        res = requests.post(url+method+i[4:],{
                            'temperature': float(info[3]),
                            'wetness':float(info[2]),
                            'par':float(info[4]),
                            'time':int(time.time())
                        })
                        '''
                        with open('sensorNode' + i + '.csv', 'a+') as csvfile:
                            writer = csv.writer(csvfile)
                            nowT = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                            writer.writerow([nowT, info[2], info[3], info[4], info[1]]) 

                        res = requests.post(url+method+i,{
                            'temperature': float(info[3]),
                            'wetness':float(info[2]),
                            'par':float(info[4]),
                            'time':int(time.time())
                        })
                        '''
                        print(res)

                
            """
            try:
                # python3 unicode
                print("Receive: {}".format( _data.encode('latin-1').decode('unicode_escape')))
            except:
                # python2
                print("Receive: {}".format( _data ))
            """
            
        except:
            print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            # print("Non-hexadecimal digit found...")
            print("Sending to server got problem...")
            print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            print("Receive: {}".format( data ))



        self.set_dio_mapping([1,0,0,0,0,0])    # TX
        # self.set_mode(MODE.STDBY)
        
        self.set_mode(MODE.SLEEP)
        
        # sleep(1)
        
        self.clear_irq_flags(TxDone=1)
        """
        data = {"id":self._id, "data":packer.ACK}
        _length, _ack = packer.Pack_Str( json.dumps(data) )

        try:
            # for python2
            ack = [int(hex(ord(c)), 0) for c in _ack]
        except:
            # for python3 
            ack = [int(hex(c), 0) for c in _ack]

        print("ACK: {}, {}".format( self._id, ack))
        self.write_payload(ack)
        """
        self.set_mode(MODE.TX)


    def on_tx_done(self):
        # print("\nTxDone")
        print('----------------------------------')
        self.set_dio_mapping([0,0,0,0,0,0])    # RX
        self.set_mode(MODE.STDBY)
        sleep(1)
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        self.clear_irq_flags(RxDone=1)


    def start(self):
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        while True:
            sleep(1)
            rssi_value = self.get_rssi_value()
            status = self.get_modem_status()
            sys.stdout.flush()
            sys.stdout.write("\r%d %d %d" % (rssi_value, status['rx_ongoing'], status['modem_clear']))

lora = LoRaGateWay(verbose=False)
args = parser.parse_args(lora)
lora.set_mode(MODE.STDBY)
lora.set_pa_config(pa_select=1)

try:
    lora.start()
except KeyboardInterrupt:
    sys.stdout.flush()
    sys.stderr.write("KeyboardInterrupt\n")
finally:
    sys.stdout.flush()
    lora.set_mode(MODE.SLEEP)
    sleep(.5)
    BOARD.teardown()



