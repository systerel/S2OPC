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

import sys, os, io
import re
from subprocess import Popen, PIPE, STDOUT
from time import sleep
import threading 

class CLI_Tester(threading.Thread):
    def __init__(self, exeName):
        threading.Thread.__init__(self, target= self.reader)
        self.done = False
        self.FAILED = []
        self.waitRe=None
        tapName = os.getenv("CK_TAP_LOG_FILE_NAME", default = "cli_self_test.tap")
        self.tap = open(tapName, "w")
        self.step = 0
        self.proc= Popen( ["stdbuf", "-oL"] + [exeName], stdin=PIPE, stdout=PIPE, stderr=STDOUT, text=True, universal_newlines=True)
        self.start()
        
    def readCheck(self, param):
        s,t=param
        print (f"---Wait for {s}")
        self.waitRe = s
        sleep(t)
        if self.waitRe != None:
            self.tap.write (f"nok {self.step} - Not found: {line}\n")
            raise Exception(f"Expression not found : {self.waitRe}")
    def write(self, s):
        print ("<<< %s"%s)
        self.proc.stdin.write(f"{s}\n")
        self.proc.stdin.flush()
        self.tap.write (f"ok {self.step} - write <{s}>\n")
    def sleep(self,t):
        print (f"--- Sleep {t}")
        sleep(t)
        self.tap.write (f"ok {self.step} - sleep({t})\n")
    def reader(self):
        try:
            #mutex.acquire()
            for line in iter(self.proc.stdout.readline, b''):
                if self.proc.poll() != None : break
                if not line:
                    #mutex.release()
                    sleep(0.1)
                    #mutex.acquire()
                else:
                    if self.waitRe:
                        if re.match(self.waitRe, line):
                            self.waitRe=None
                            # RE found
                            line = line.replace("\n","")
                            print("!!! " + line.rstrip())
                            self.tap.write (f"ok {self.step} - Found {line}\n")
                        else:
                            # Still waiting for RE
                            print("??? " + line.rstrip())
                    else:
                        # Not waiting. Simple output
                        print(">>> " + line.rstrip())
            #mutex.release()
        finally: self.done = True
    
    def scenExec(self, scenario):
        
        self.tap.write(f"1..{len(scenario)}\n")
        stat=None
        for cmd, param in scenario:
            self.step += 1
            testName = "STEP %d: %s(%s)"%(self.step, cmd,param)
            stat = self.proc.poll()
            
            if stat is not None:
                print("Process exited with status =%s", stat)
                break
            try:
                if cmd == "sleep": cmd = self.sleep
                if cmd == "write": cmd = self.write
                if cmd == "read": cmd = self.readCheck
                cmd(param)
            except:
                self.FAILED.append(f"{testName} failed")
        if stat == None:
            stat = self.proc.poll()
        self.tap.flush()
        return stat
    
if __name__=='__main__':
    try:
        tester = CLI_Tester("./S2OPC_CLI_PubSub_Server")
    except Exception as e:
        print(e, file =sys.stderr)
        
    scenario=[("sleep",2), 
              ("write","info"), 
              ("read",(r"Publisher running *: *NO",1.0)),
              ("write","info"), 
              ("read",(r"Subscriber *:.*DISABLED",1.0)),
              
              ("write","sub start"), 
              ("write","info"), 
              ("read",(r"Subscriber *:.*OPERATIONAL",1.0)),
              ("sleep",2), 
              ("write","info"), 
              ("read",(r".*Group.*WriterId *= *20.*: *ERROR",1.0)),
              
              ("write","pub start"), 
              ("write","info"), 
              ("read",(r"Publisher running *: *YES",1.0)),
              ("sleep",2), 
              ("write","info"), 
              ("read",(r".*Group.*WriterId *= *20.*: *OPERATIONAL",1.0)),
              
              ("write","pub stop"), 
              ("sleep",2), 
              ("write","info"), 
              ("read",(r"Subscriber *:.*OPERATIONAL",1.0)),
              ("sleep",2), 
              ("write","info"), 
              ("read",(r".*Group.*WriterId *= *20.*: *ERROR",1.0)),
              
              ("write","quit"),
              ("sleep",2)
              ]
    
    stat = tester.scenExec(scenario)
    if stat != 0 : 
        tester.FAILED.append(f"Exit code unexpected: {stat}")
    if tester.FAILED:
        print (f"Test failed:{tester.FAILED}")
        sys.exit(-1)
    print (f"Test OK!")
    