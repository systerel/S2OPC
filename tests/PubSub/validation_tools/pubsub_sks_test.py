#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import os
import argparse
import pathlib
import sys
import traceback
from abc import ABC, abstractmethod
import atexit
import subprocess
import time
from tap_logger import TapLogger
from pubsub_server import PubSubServer
from pubsub_server_test import helpConfigurationChangeAndStart, helpTestSetValue


# Timeout for SKS Client to activate session
CLIENT_TIMEOUT_ACTIVATE_SESSION = 10 # 10s
SERVER_TIMEOUT_SHUTDOWN = 10 # 10s


# Debug Log
def log_test(msg):
    print(' /!\ Integration Test : %s' % msg, flush=True)


def Test_Sleep(s):
    log_test('Wait %d secondes' % s)
    time.sleep(s)
    log_test('.......... finish waiting')




class AbstractBenchmarkManager(ABC):


    @abstractmethod
    def stop_all(self):
        None

    @abstractmethod
    def start_sks_master(self, restart=False):
        None

    @abstractmethod
    def stop_sks_master(self):
        None

    @abstractmethod
    def start_sks_slave(self, num):
        None

    @abstractmethod
    def stop_sks_slave(self, num):
        None

    @abstractmethod
    def start_publisher(self):
        None

    @abstractmethod
    def pause_publisher(self):
        None

    @abstractmethod
    def stop_publisher(self):
        None

    @abstractmethod
    def start_subscriber(self):
        None

    @abstractmethod
    def pause_subscriber(self):
        None

    @abstractmethod
    def stop_subscriber(self):
        None


class LocalBenchmarkManager(AbstractBenchmarkManager):

    def __init__(self, bindir, max_slave, port_pub, port_sub):
        binary_directory = bindir
        binary_sks = binary_directory.joinpath('toolkit_demo_sks')
        binary_pubsub_server = binary_directory.joinpath('pubsub_server')
        self.command_sks_master = [str(binary_sks), 'master']
        self.command_sks_slave = [str(binary_sks), 'slave']
        self.command_publisher = [str(binary_pubsub_server), 'opc.tcp://localhost:%s' % port_pub]
        self.command_subscriber = [str(binary_pubsub_server), 'opc.tcp://localhost:%s' % port_sub]

        self.max_slave = max_slave

        # list of subprocess
        self.all_processes = []
        self.sks_master = None
        self.sks_slave = dict()
        self.publisher = None
        self.subscriber = None

        self.output = []
        self.fileCount = 0

    def __start_proc__(self, cmd, filename, show=False):
        #proc = subprocess.Popen(cmd)
        #proc = subprocess.Popen(cmd, shell = True, stdin = None, stdout = subprocess.PIPE, stderr = subprocess.PIPE, encoding = 'utf8')
        show = True

        if show == True:
            print("Popen with output")
            proc = subprocess.Popen(cmd)
        else:

            print("Popen without output")
            # open a file to redirect stdout
            self.fileCount = self.fileCount + 1
            outfile = open('%s_%s' % (filename, self.fileCount), "w")
            self.output.append(outfile)

            # run the command
            proc = subprocess.Popen(cmd, stdout = outfile, encoding = 'utf8')

        self.all_processes.append(proc)

        return proc

    def __stop_proc__(self, proc):
        proc.kill()
        self.all_processes.remove(proc)

    def stop_all(self):
        timeout_sec = 2 # 2s
        i = 0
        for p in self.all_processes:
            i = i + 1
            p_sec = 0
            #for second in range(timeout_sec):
            #    if p.poll() == None:
            #        time.sleep(1)
            #        p_sec += 1
            #if p_sec >= timeout_sec:
            if True:
                log_test("Kill proc %d" % i)
                p.kill()
                #log_test("Wait until proc is killed %d" % i)
                p.wait()
                #log_test("Proc %d killed" % i)
        self.all_processes = []
        self.sks_master = None
        self.sks_slave = dict()
        self.publisher = None
        self.subscriber = None
        for output in self.output:
            print("close file")
            output.close()
        self.output = []

    def start_sks_master(self, restart=False):
        log_test("Start SKS Master")
        if self.sks_master is None:
            master = self.command_sks_master.copy()
            if restart:
                master.append('--restart')
            self.sks_master = self.__start_proc__(master, 'master', True)
        else:
            log_test("Warning : SKS Master is already running")

    def stop_sks_master(self):
        log_test("Stop SKS Master")
        if not self.sks_master is None:
            self.__stop_proc__(self.sks_master)
            self.sks_master = None


    def start_sks_slave(self, num):
        log_test("Start SKS Slave %i" % num)

        if num < 1 or num > self.max_slave:
            log_test("Error : %d is not a valid id for Slave. It should be between 1 and %d" % (num, self.max_slave))
            return

        if not num in self.sks_slave :
            slave = self.command_sks_slave.copy()
            slave.append(str(num))
            self.sks_slave[num] = self.__start_proc__(slave, 'slave_%s' % num)
        else:
            log_test("Warning : SKS slave %d is already running" % num )

    def stop_sks_slave(self, num):
        log_test("Stop SKS Slave %d" % num)
        if num < 1 or num > self.max_slave:
            log_test("Error : %d is not a valid id for Slave. It should be between 1 and %d" % (num, self.max_slave))
            return

        if num in self.sks_slave :
            self.__stop_proc__(self.sks_slave[num])
            del self.sks_slave[num]

    def start_publisher(self):
        log_test("Start Publisher")
        if self.publisher is None:
            self.publisher = self.__start_proc__(self.command_publisher, 'publisher')
        else:
            log_test("Warning : Publisher is already running")

    def pause_publisher(self):
        None

    def stop_publisher(self):
        log_test("Stop SKS Publisher")
        if not self.publisher is None:
            self.__stop_proc__(self.publisher)
            self.publisher = None

    def start_subscriber(self):
        log_test("Start Subscriber")
        if self.subscriber is None:
            self.subscriber = self.__start_proc__(self.command_subscriber, 'subscriber', True)
        else:
            log_test("Warning : Subscriber is already running")

    def pause_subscriber(self):
        None

    def stop_subscriber(self):
        log_test("Stop SKS Subscriber")
        if not self.subscriber is None:
            self.__stop_proc__(self.subscriber)
            self.subscriber = None


# To be called on exit to close all process
def killchild():
    global localbenchmark 
    localbenchmark.stop_all()

# Set variables in a PubSub server
def __set_variables(pubsubserver, nid_bool, val_bool, nid_uint16, val_uint16, nid_int, val_int):
    
    helpTestSetValue(pubsubserver, nid_bool, val_bool, logger)
    helpTestSetValue(pubsubserver, nid_uint16, val_uint16, logger)
    helpTestSetValue(pubsubserver, nid_int, val_int, logger)
    
# Test variables in a PubSub server
def __test_variables(name, pubsubserver, nid_bool, val_bool, nid_uint16, val_uint16, nid_int, val_int):
    read_bool = pubsubserver.getValue(nid_bool)
    read_uint16 = pubsubserver.getValue(nid_uint16)
    read_int = pubsubserver.getValue(nid_int)
    logger.add_test('Test %s bool change. Expected %s, read %s' % (name, val_bool, read_bool), val_bool == read_bool)
    logger.add_test('Test %s uint16 change. Expected %s, read %s' % (name, val_uint16, read_uint16), val_uint16 == read_uint16)
    logger.add_test('Test %s int change. Expected %s, read %s' % (name, val_int, read_int), val_int == read_int)


# Load PubSub Server configuration
def load_pubsubserver_configuration(uri, xml):

    logger.add_test('Load PubSub server configuration', True)
    
    pubsub_server = PubSubServer(uri, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)
    pubsub_server.connect()
    helpConfigurationChangeAndStart(pubsub_server, xml, logger)

# Test that there is no exchange of Address Space value
def test_no_adresse_space_exchange():

    logger.add_test('Test no Adresse Space exchange', True)
    
    publisher_server = PubSubServer(PUBLISHER_URI, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)
    subscriber_server = PubSubServer(SUBSCRIBER_URI, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)

    # secure channel connection
    publisher_server.connect()
    subscriber_server.connect()

    # Set Subscriber and Publisher variables then checks Subscriber variables
    __set_variables(subscriber_server, NID_SUB_BOOL, False, NID_SUB_UINT16, 2002, NID_SUB_INT, 1311)
    __set_variables(publisher_server, NID_PUB_BOOL, True, NID_PUB_UINT16, 1422, NID_PUB_INT, 3003)
    time.sleep(1)
    __test_variables('Subscriber', subscriber_server, NID_SUB_BOOL, False, NID_SUB_UINT16, 2002, NID_SUB_INT, 1311)
    __test_variables('Publisher', publisher_server, NID_PUB_BOOL, True, NID_PUB_UINT16, 1422, NID_PUB_INT, 3003)

def test_adresse_space_exchange():

    logger.add_test('Test Adresse Space exchange', True)

    publisher_server = PubSubServer(PUBLISHER_URI, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)
    subscriber_server = PubSubServer(SUBSCRIBER_URI, NID_CONFIGURATION, NID_START_STOP, NID_STATUS)

    # secure channel connection
    publisher_server.connect()
    subscriber_server.connect()

    # Test 1
    __set_variables(publisher_server, NID_PUB_BOOL, True, NID_PUB_UINT16, 4561, NID_PUB_INT, 123456)
    time.sleep(1)
    __test_variables('Subscriber', subscriber_server, NID_SUB_BOOL, True, NID_SUB_UINT16, 4561, NID_SUB_INT, 123456)
    
    # Test 2
    __set_variables(publisher_server, NID_PUB_BOOL, False, NID_PUB_UINT16, 1234, NID_PUB_INT, 654123)
    time.sleep(1)
    __test_variables('Subscriber', subscriber_server, NID_SUB_BOOL, False, NID_SUB_UINT16, 1234, NID_SUB_INT, 654123)


    
##################################################
# CONFIGURATION
##################################################


XML_PUBLISHER_ONLY = """<PubSub publisherId="1">
    <connection address="mqtts://127.0.0.1:1883" mode="publisher">
        <message id="1" publishingInterval="100" version="1" securityMode="signAndEncrypt">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/server_2k_cert.der" />
            <skserver endpointUrl="opc.tcp://localhost:4842" serverCertPath="./server_public/server_2k_cert.der" />
            <skserver endpointUrl="opc.tcp://localhost:4843" serverCertPath="./server_public/server_2k_cert.der" />
            <variable nodeId="ns=1;s=PubBool" displayName="pubVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=PubUInt16" displayName="pubVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=PubInt" displayName="pubVarInt" dataType="Int64" />
        </message>
    </connection>
</PubSub>"""

XML_SUBSCRIBER_ONLY = """<PubSub>
    <connection address="mqtts://127.0.0.1:1883" mode="subscriber">
        <message id="1" publishingInterval="100" version="1" publisherId="1" securityMode="signAndEncrypt">
            <skserver endpointUrl="opc.tcp://localhost:4841" serverCertPath="./server_public/server_2k_cert.der" />
            <skserver endpointUrl="opc.tcp://localhost:4842" serverCertPath="./server_public/server_2k_cert.der" />
            <skserver endpointUrl="opc.tcp://localhost:4843" serverCertPath="./server_public/server_2k_cert.der" />
            <variable nodeId="ns=1;s=SubBool" displayName="subVarBool" dataType="Boolean" />
            <variable nodeId="ns=1;s=SubUInt16" displayName="subVarUInt16" dataType="UInt16" />
            <variable nodeId="ns=1;s=SubInt" displayName="subVarInt" dataType="Int64" />
        </message>
    </connection>
</PubSub>"""

PUBLISHER_URI = 'opc.tcp://localhost:4851'
SUBSCRIBER_URI = 'opc.tcp://localhost:4852'
NID_CONFIGURATION = u"ns=1;s=PubSubConfiguration"
NID_START_STOP = u"ns=1;s=PubSubStartStop"
NID_STATUS = u"ns=1;s=PubSubStatus"

# Subscriber Node
NID_SUB_BOOL = u"ns=1;s=SubBool"
NID_SUB_UINT16 = u"ns=1;s=SubUInt16"
NID_SUB_INT = u"ns=1;s=SubInt"

# Publisher Node
NID_PUB_BOOL = u"ns=1;s=PubBool"
NID_PUB_UINT16 = u"ns=1;s=PubUInt16"
NID_PUB_INT = u"ns=1;s=PubInt"


#PurePosixPath('/users/vla/Private/03_DEV/INGOPCS/build/bin/')
#BINARY_DIRECTORY = PurePosixPath('/users/aurelien/git/S2OPC_new/build/bin/')
#localbenchmark = LocalBenchmarkManager(bindir=BINARY_DIRECTORY, max_slave=2, port_pub='4851', port_sub='4852')

logger = TapLogger("sks_redundancy_test.tap")

####################################################################################################
# TC 1 :
#   Fonctionnement du SKS en mode nominal.
#     Si le maître et les deux esclaves sont présents tous les deux,
#     les modules récupèrent les clés sur le maître (la vérification pourra être basée sur les logs)
####################################################################################################
def integration_TC1(benchmark):

    logger.add_test('=============== Test Case 1 ===============', True)

    # Start Master, Slave 1 and Slave2
    benchmark.start_sks_master()
    Test_Sleep(5)
    benchmark.start_sks_slave(1)
    Test_Sleep(5)
    benchmark.start_sks_slave(2)
    Test_Sleep(5)
    
    # Start Publisher and Subscriber
    benchmark.start_publisher()
    Test_Sleep(3)
    benchmark.start_subscriber()
    Test_Sleep(3)
    load_pubsubserver_configuration(PUBLISHER_URI, XML_PUBLISHER_ONLY)
    load_pubsubserver_configuration(SUBSCRIBER_URI, XML_SUBSCRIBER_ONLY)
    Test_Sleep(3)
    
    # Test data exchange
    test_adresse_space_exchange()

    # Stop all process
    benchmark.stop_all()
    

####################################################################################################
# TC 2 :
#   Fonctionnement du SKS en mode dégradé.
#     Si le maître tombe, les modules récupèrent les clés sur le premier des deux esclaves
####################################################################################################
def integration_TC2(benchmark):

    logger.add_test('=============== Test Case 2 ===============', True)
    
    # Start Master, Slave 1 and Slave2
    benchmark.start_sks_master()
    Test_Sleep(5)
    benchmark.start_sks_slave(1)
    Test_Sleep(5)
    benchmark.start_sks_slave(2)
    Test_Sleep(5)
    
    # Stop Master
    benchmark.stop_sks_master()
    Test_Sleep(SERVER_TIMEOUT_SHUTDOWN)

    # Start Publisher and Subscriber
    benchmark.start_publisher()
    benchmark.start_subscriber()
    Test_Sleep(3)
    load_pubsubserver_configuration(PUBLISHER_URI, XML_PUBLISHER_ONLY)
    load_pubsubserver_configuration(SUBSCRIBER_URI, XML_SUBSCRIBER_ONLY)
    Test_Sleep(3)

    # Test no data exchange
    test_no_adresse_space_exchange()
    
    # Publisher and subscriber try to get Keys from Master.
    # Waits until their timeout
    Test_Sleep(CLIENT_TIMEOUT_ACTIVATE_SESSION)
    
    # Publisher and Subscriber should have getted Keys from Slave 1
    # Test data exchange
    test_adresse_space_exchange()
    
    # Stop all process
    benchmark.stop_all()

####################################################################################################
# TC 3 :
#   Fonctionnement du SKS en mode dégradé.
#     Si le maître et le premier esclave tombent,
#     les modules récupèrent les clés sur le second des deux esclaves
####################################################################################################
def integration_TC3(benchmark):
    
    logger.add_test('=============== Test Case 3 ===============', True)

    # Start Master, Slave 1 and Slave2
    
    benchmark.start_sks_master()
    Test_Sleep(5)
    
    benchmark.start_sks_slave(1)
    Test_Sleep(5)

    benchmark.start_sks_slave(2)
    Test_Sleep(5)
    
    # Stop Master and Slave 1
    
    benchmark.stop_sks_master()
    benchmark.stop_sks_slave(1)
    Test_Sleep(SERVER_TIMEOUT_SHUTDOWN)

    # Start Publisher and Subscriber
    
    benchmark.start_publisher()
    benchmark.start_subscriber()
    Test_Sleep(3)
    load_pubsubserver_configuration(PUBLISHER_URI, XML_PUBLISHER_ONLY)
    load_pubsubserver_configuration(SUBSCRIBER_URI, XML_SUBSCRIBER_ONLY)
    Test_Sleep(3)

    # Test no data exchange
    test_no_adresse_space_exchange()
    
    # Publisher and subscriber try to get Keys from Master.
    # Waits until their timeout
    Test_Sleep(CLIENT_TIMEOUT_ACTIVATE_SESSION)

    # Test no data exchange
    test_no_adresse_space_exchange()
    
    # Publisher and subscriber try to get Keys from Slave 1.
    # Waits until their timeout
    Test_Sleep(CLIENT_TIMEOUT_ACTIVATE_SESSION)


    # Publisher and Subscriber should have getted Keys from Slave 2
    # Test data exchange
    test_adresse_space_exchange()
    
    # Stop all process
    benchmark.stop_all()

####################################################################################################
# TC 4 :
#   Fonctionnement du SKS en mode dégradé.
#     Si le maître tombe puis redémarre, il peut à nouveau fournir ses clés aux modules.
####################################################################################################
def integration_TC4(benchmark):

    logger.add_test('=============== Test Case 4 ===============', True)

    # Start Master, Slave 1 and Slave2
    benchmark.start_sks_master()
    Test_Sleep(5)
    benchmark.start_sks_slave(1)
    Test_Sleep(5)
    benchmark.start_sks_slave(2)
    Test_Sleep(5)
    
    # Start Publisher
    #   => Publisher get keys from master
    benchmark.start_publisher()
    Test_Sleep(3)
    load_pubsubserver_configuration(PUBLISHER_URI, XML_PUBLISHER_ONLY)
    Test_Sleep(3)

    # Stop Master
    benchmark.stop_sks_master()
    Test_Sleep(SERVER_TIMEOUT_SHUTDOWN)
    
    # Restart Master and stop Slaves
    #   => Master get keys from slave 1
    benchmark.start_sks_master(restart=True)
    Test_Sleep(5)
    benchmark.stop_sks_slave(1)
    benchmark.stop_sks_slave(2)
    Test_Sleep(SERVER_TIMEOUT_SHUTDOWN)
    
    # Start Subscriber
    #   => Subscriber get keys from master
    benchmark.start_subscriber()
    Test_Sleep(3)
    load_pubsubserver_configuration(SUBSCRIBER_URI, XML_SUBSCRIBER_ONLY)
    Test_Sleep(3)

    # Test data exchange
    test_adresse_space_exchange()

    # Stop all process
    benchmark.stop_all()

####################################################################################################
# TC 5 :
#   Fonctionnement du SKS en mode dégradé.
#     Cas identique a TC4 mais Publisher recupère les clés par slave 2
####################################################################################################
def integration_TC5(benchmark):

    logger.add_test('=============== Test Case 5 ===============', True)

    # Start Master, Slave 1 and Slave2
    benchmark.start_sks_master()
    Test_Sleep(5)
    benchmark.start_sks_slave(1)
    Test_Sleep(5)
    benchmark.start_sks_slave(2)
    Test_Sleep(5)
    
    # Stop Master and Slave 1
    benchmark.stop_sks_master()
    benchmark.stop_sks_slave(1)
    Test_Sleep(SERVER_TIMEOUT_SHUTDOWN)
    
    # Start Publisher
    #   => Publisher get keys from slave 2
    benchmark.start_publisher()
    Test_Sleep(3)
    load_pubsubserver_configuration(PUBLISHER_URI, XML_PUBLISHER_ONLY)
    Test_Sleep(3)
    # Get Keys from Master timeout
    Test_Sleep(CLIENT_TIMEOUT_ACTIVATE_SESSION)
    # Get Keys from Slave 1 timeout
    Test_Sleep(CLIENT_TIMEOUT_ACTIVATE_SESSION)
    
    # Restart Master
    #   => Master get keys from slave 2
    benchmark.start_sks_master(restart=True)
    Test_Sleep(5)
    
    # Master try to get Keys from Slave 1.
    # Waits until timeout
    Test_Sleep(CLIENT_TIMEOUT_ACTIVATE_SESSION)

    # Stop Slave 2
    benchmark.stop_sks_slave(2)
    Test_Sleep(SERVER_TIMEOUT_SHUTDOWN)
    
    # Start Subscriber
    #   => Subscriber get keys from master
    benchmark.start_subscriber()
    Test_Sleep(3)
    load_pubsubserver_configuration(SUBSCRIBER_URI, XML_SUBSCRIBER_ONLY)
    Test_Sleep(3)

    # Test data exchange
    test_adresse_space_exchange()

    # Stop all process
    benchmark.stop_all()


def execute_tests(lbenchmark):

    # Start Local Benchmark Test
    try:
        log_test(" ---> TEST INTEGRATION TC1 <---")
        integration_TC1(lbenchmark)
    
        log_test(" ---> TEST INTEGRATION TC2 <---")
        integration_TC2(lbenchmark)
    
        log_test(" ---> TEST INTEGRATION TC3 <---")
        integration_TC3(lbenchmark)
    
        log_test(" ---> TEST INTEGRATION TC4 <---")
        integration_TC4(lbenchmark)
    
        log_test(" ---> TEST INTEGRATION TC5 <---")
        integration_TC5(lbenchmark)
    
    except:
        traceback.print_exc(file=sys.stdout)
        
    logger.finalize_report()
    
    log_test(" ---> EXIT <---")
    killchild()
    sys.exit(0)
    log_test(" ---> END OF SCRIPT <---")


if __name__=='__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--binary_dir', required=True, action='store', default="",
                           help='Directory containing binaries to test.')
    args = argparser.parse_args()
    
    BINARY_DIRECTORY=pathlib.PurePosixPath(args.binary_dir)
    localbenchmark = LocalBenchmarkManager(bindir=BINARY_DIRECTORY, max_slave=2, port_pub='4851', port_sub='4852')
    
    # Register in call on exist
    atexit.register(killchild)
    
    execute_tests(localbenchmark);

