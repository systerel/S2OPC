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
import tap_logger
class CLI_Tester(threading.Thread):
    """
    This class starts a process (Process under test). It also starts a thread that captures output (see reader()).
    See scenExec() for details on how to execute a test.
    """
    def __init__(self, exeName):
        """
        @param exeName The path to the process under test to start.
        """
        threading.Thread.__init__(self, target= self.reader)
        self.done = False
        self.FAILED = []
        self.waitRe=None
        tapName = os.getenv("CK_TAP_LOG_FILE_NAME", default = "cli_self_test.tap")
        self.tap = tap_logger.TapLogger(tapName, verbose=False)
        # Start the process and capture both stdout and stderr into a PIPE
        # "stdbuf -oL" option is used to prevent OS from buffering input in PIPE
        self.proc= Popen( ["stdbuf", "-oL"] + [exeName], stdin=PIPE, stdout=PIPE, stderr=STDOUT, text=True, universal_newlines=True)
        self.start()
        
    def readCheck(self, param):
        s,t=param
        print (f"---Wait for {s}")
        self.waitRe = s
        sleep(t)
        if self.waitRe != None:
            self.tap.add_test(f"Not found: {self.waitRe}", False)
            raise Exception(f"Expression not found : {self.waitRe}")
    def write(self, s):
        print ("<<< %s"%s)
        self.proc.stdin.write(f"{s}\n")
        self.proc.stdin.flush()
        self.tap.add_test(f"write <{s}>", True)
    def sleep(self,t):
        print (f"--- Sleep {t}")
        sleep(t)
        self.tap.add_test(f"sleep({t})", True)
    def readInfoCheck(self, param):
        s,t=param
        print(f"---Wait for {s}")
        self.waitRe = s
        self.write("info")
        sleep(t)
        if self.waitRe != None:
            self.tap.add_test(f"Not found: {self.waitRe}", False)
            raise Exception(f"Expression not found : {self.waitRe}")
    def reader(self):
        """
        This function is run in a dedicated thread and polls the output(stdout & stderr) of process under test.
        It parses the lines to detect the currently expected result (in self.waitRe).
        It returns when the external process (self.proc) is terminated.
        """
        try:
            for line in iter(self.proc.stdout.readline, b''):
                
                # Check if the Process under test is still running. Exit thread otherwise.
                if self.proc.poll() != None : break
                if not line:
                    sleep(0.1)
                else:
                    if self.waitRe:
                        # Currently expecting something, check if the expected is obtained
                        if re.match(self.waitRe, line):
                            self.waitRe=None
                            # Result found.
                            line = line.replace("\n","")
                            print("!!! " + line.rstrip())
                            self.tap.add_test(f"Found {line}",True)
                        else:
                            # Still waiting for result
                            print("??? " + line.rstrip())
                    else:
                        # Not waiting. Simple output
                        print(">>> " + line.rstrip())
        finally: self.done = True
    
    def scenExec(self, scenario):
        """
            Executes a scenario on the Process under test (PUT)
            @param scenario a [(cmd:str, param)]
            cmd is a string providing the action to execute on PUT. Currently supporting 'sleep': 
              - 'sleep': param is an int (the sleep duration in second) 
              - 'write': param is a str (the text to send in PUT input PIPE)
              - 'read': param is a (exp:re, timeout:int) with: exp= regular expression (the expected output in 
              PUT output PIPE) and timeout the timeout for reception (in second). An exception is raised if 
              'exp' is not found within 'timeout' seconds
              - 'readInfo': param is a (exp:re, timeout:int) with: exp= regular expression (the expected output in 
              PUT output PIPE) and timeout the timeout for reception (in second). An exception is raised if 
              'exp' is not found within 'timeout' seconds.
        """
        stat=None
        for cmd, param in scenario:
            testName = "STEP %d: %s(%s)"%(self.tap.nb_tests, cmd,param)
            stat = self.proc.poll()
            
            if stat is not None:
                print("Process exited with status =%s", stat)
                break
            try:
                if cmd == "sleep": cmd = self.sleep
                if cmd == "write": cmd = self.write
                if cmd == "read": cmd = self.readCheck
                if cmd == "readInfo": cmd = self.readInfoCheck
                cmd(param)
            except:
                self.FAILED.append(f"{testName} failed")
        if stat == None:
            stat = self.proc.poll()
        self.tap.finalize_report()
        return stat
    
if __name__=='__main__':
    try:
        tester = CLI_Tester("./S2OPC_CLI_PubSub_Server")
    except Exception as e:
        print(e, file =sys.stderr)
        
    scenario=[("sleep",2), 
              ("readInfo",(r"Publisher running *: *NO",2.0)),
              ("readInfo",(r"Subscriber *:.*DISABLED",2.0)),
              
              ("write","sub start"), 
              ("readInfo",(r"Subscriber *:.*OPERATIONAL",2.0)),
              ("sleep",2), 
              ("readInfo",(r".*Group.*WriterId *= *20.*: *ERROR",2.0)),
              
              ("write","pub start"), 
              ("readInfo",(r"Publisher running *: *YES",2.0)),
              ("sleep",2), 
              ("readInfo",(r".*Group.*WriterId *= *20.*: *OPERATIONAL",2.0)),
              
              ("write","pub stop"), 
              ("sleep",2), 
              ("readInfo",(r"Subscriber *:.*OPERATIONAL",2.0)),
              ("sleep",2), 
              ("readInfo",(r".*Group.*WriterId *= *20.*: *ERROR",2.0)),
              
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
    