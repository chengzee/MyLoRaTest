#!/usr/bin/env python3
# -*- coding: utf8 -*-
""" A simple beacon transmitter class to send a 1-byte message (0x0f) in regular time intervals. """
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

# in python 2
import urlparse
# in python 3
# import urllib.parse
import requests
url="http://icmems.ml:3000"
method='/api/sendDatas/'

BOARD.setup()

parser = LoRaArgumentParser("Continous LoRa receiver.")
# python2
try:
    import sys
    reload(sys)
    sys.setdefaultencoding('utf-8')
except:
    pass

# Create Node list
Nodes = 3      # number of Nodes
node_list = []
for n in range(Nodes):
    node_list.append("Node"+str(n+1))

number_node = 0            # No. node
    
class LoRaGatewayClassC(LoRa):
    def __init__(self, verbose=False):
        super(LoRaGatewayClassC, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        #self.set_dio_mapping([0,0,0,0,0,0])    # RX
        self.set_dio_mapping([1,0,0,0,0,0])    # TX
        self._id = "Gateway_01"
        self.rx_done = False
        
    def on_rx_done(self):
        print("\nRxDone")
        # analyze receive
        payload = self.read_payload(nocheck=True)
        data = ''.join([chr(c) for c in payload])
        print("Time: {}".format(str(time.ctime())))
        print("Raw RX: {}".format(data))
        # split receive
        info = data.split(",")
        # print(len(info))
        try:
            if(len(info)==5):
                if(info[0] == node_list[number_node]):
                    # sensor_number is for database
                    sensor_number = number_node+1
                    print("Time: {}".format( str(time.ctime() )))
                    print("This is {}.".format(info[0]))
                    print("temperature:{}".format(float(info[3])))
                    print("humidity:{}".format(float(info[2])))
                    print("PAR:{}".format(float(info[4])))
                    res = requests.post(url+method+str(sensor_number),{
                        'temperature': float(info[3]),
                        'wetness':float(info[2]),
                        'par':float(info[4])
                    })
                    print(res)
            if(len(info)==4):
                if(info[0] == node_list[number_node]):
                    # sensor_number is for database
                    sensor_number = number_node+1
                    print("Time: {}".format( str(time.ctime() )))
                    print("This is {}.".format(info[0]))
                    print("temperature:{}".format(float(info[3])))
                    print("humidity:{}".format(float(info[2])))
                    res = requests.post(url+method+str(sensor_number),{
                        'temperature': float(info[3]),
                        'wetness':float(info[2])
                    })
                    print(res)
             
        except:
            print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            print("Non-hexadecimal digit found...")
            print("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            print("Receive: {}".format( data))

        # set TX
        self.rx_done = True
        # comment it will receive countinous
        self.set_dio_mapping([1,0,0,0,0,0])    # TX
        self.set_mode(MODE.STDBY)
        self.clear_irq_flags(TxDone=1)


    def on_tx_done(self):
        print("\nTxDone")
        # set RX
        self.set_dio_mapping([0,0,0,0,0,0])    # RX
        sleep(1)
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        self.clear_irq_flags(RxDone=1)

    def start(self):
        while True:
            global number_node
            
            print('----------------------------------')
            sleep(1)

            try:
                # initialize request
                if number_node == len(node_list):
                    number_node = 0
                # request
                request_msg = node_list[number_node]
            #    rawinput = raw_input(">>> ")
            #except NameError:
            #    rawinput = input(">>> ")
                print(request_msg)
            except KeyboardInterrupt:
                lora.set_mode(MODE.SLEEP)
                sleep(.5)
                BOARD.teardown()
                exit()

            if len(request_msg) < 200:
                """
                # data = {"id":self._id, "data":rawinput}
                # _length, _payload = packer.Pack_Str( json.dumps(data) )
                data = {"id":self._id, "data":request_msg}
                _length, _payload = packer.Pack_Str( json.dumps(data) )
                
                try:
                    # for python2
                    data = [int(hex(ord(c)), 0) for c in _payload]
                except:
                    # for python3 
                    data = [int(hex(c), 0) for c in _payload]
                """
                try:
                    # for python2
                    data = [int(hex(ord(c)), 0) for c in request_msg]
                except:
                    # for python3 
                    data = [int(hex(c), 0) for c in request_msg]
                    
                for i in range(3):
                    if self.rx_done is True:
                        self.rx_done = False
                        break
                    else:
                        self.set_mode(MODE.SLEEP)
                        self.set_dio_mapping([1,0,0,0,0,0])    # TX
                        sleep(.5)
                        lora.set_pa_config(pa_select=1)
                        self.clear_irq_flags(TxDone=1)
                        self.set_mode(MODE.STDBY)
                        sleep(.5)
                        # print("Raw TX: {}".format(data))
                        print("int(hex(request_msg)) TX: {}".format( data ))
                        print("request_msg TX: {}".format( request_msg ))
                        self.write_payload(data)
                        self.set_mode(MODE.TX)
                        """
                        ## ALOHA(1~3) ## on_tx_done
                        t = i*i + int(np.random.random() * float(_length))
                        print("ALOHA Waiting: {}".format( t))
                        """
                        t = 10
                        print("Waiting:{}".format(t))
                        sleep(t)
                # request to next node
                # number_node = number_node + 1
                number_node += 1


lora = LoRaGatewayClassC()
args = parser.parse_args(lora)
lora.set_mode(MODE.STDBY)
lora.set_pa_config(pa_select=1)

try:
    lora.start()
except KeyboardInterrupt:
    sys.stdout.flush()
    print("")
    sys.stderr.write("KeyboardInterrupt\n")
finally:
    sys.stdout.flush()
    lora.set_mode(MODE.SLEEP)
    sleep(.5)
    BOARD.teardown()

