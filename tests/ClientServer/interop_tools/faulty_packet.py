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

# This script is designed to test the non-regression of an S2OPC server when handling erroneous packets
# after the connection and session have been established.
#
# The tests are presented in the form of scenarios and expect servers with specific configurations.
#
# Test scenarios 1, 2, 3, and 4 expect a server with the following characteristics:
# - Ability to connect with an anonymous user in None security mode
# - The anonymous user does not require special rights on the address space
#
# Test scenario 5 expects a server that:
# - Rejects unsecured access, i.e., None security mode
#
# Notes: The OPC UA standard is backward compatible. In this context, the offsets used to obtain identifiers,
# status codes, and other information are hardcoded for time-saving purposes, as they are not expected to change.

import socket
import signal
import sys
import argparse
import re
from random import randint
from random import seed
from tap_logger import TapLogger

# Host, port and url of the server running S2OPC
HOST = "127.0.0.1"
PORT = 4841

# Timeout after which a connection should be closed, useful when no response is given by the server for a browse request
TIMEOUT = 1

# Messages to setup the OPCUA connection and to close the connection
HELLO_MSG = b"\x48\x45\x4c\x46\x3b\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xfb\xff\x04\x00\x05\x00\x00\x00\x1b\x00\x00\x00\x6f\x70\x63\x2e\x74\x63\x70\x3a\x2f\x2f\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x30\x2e\x31\x3a\x34\x38\x34\x31"
OPEN_SECURE_CHANNEL_MSG = b"\x4f\x50\x4e\x46\x84\x00\x00\x00\x00\x00\x00\x00\x2f\x00\x00\x00\x68\x74\x74\x70\x3a\x2f\x2f\x6f\x70\x63\x66\x6f\x75\x6e\x64\x61\x74\x69\x6f\x6e\x2e\x6f\x72\x67\x2f\x55\x41\x2f\x53\x65\x63\x75\x72\x69\x74\x79\x50\x6f\x6c\x69\x63\x79\x23\x4e\x6f\x6e\x65\xff\xff\xff\xff\xff\xff\xff\xff\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\xbe\x01\x00\x00\x81\x12\xcf\x1e\x1e\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xff\xff\xff\xff\x60\xea\x00\x00"
CREATE_SESSION_MSG = b"\x4d\x53\x47\x46\xc7\x00\x00\x00\xbf\xab\x84\x6a\x11\x5f\xce\x37\x02\x00\x00\x00\x02\x00\x00\x00\x01\x00\xcd\x01\x00\x00\xc2\x89\xcf\x1e\x1e\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\x13\x00\x00\x00\x75\x72\x6e\x3a\x53\x32\x4f\x50\x43\x3a\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\xff\xff\xff\xff\x02\x10\x00\x00\x00\x53\x32\x4f\x50\x43\x5f\x44\x65\x6d\x6f\x43\x6c\x69\x65\x6e\x74\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x1b\x00\x00\x00\x6f\x70\x63\x2e\x74\x63\x70\x3a\x2f\x2f\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x30\x2e\x31\x3a\x34\x38\x34\x31\x13\x00\x00\x00\x53\x32\x4f\x50\x43\x5f\x63\x6c\x69\x65\x6e\x74\x5f\x73\x65\x73\x73\x69\x6f\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x4c\xed\x40\xfb\xff\x04\x00"
ACTIVATE_SESSION_MSG = b"\x4d\x53\x47\x46\x6c\x00\x00\x00\xbf\xab\x84\x6a\x11\x5f\xce\x37\x03\x00\x00\x00\x03\x00\x00\x00\x01\x00\xd3\x01\x02\x00\x00\x3a\x04\x24\x3f\xd9\x2c\xd0\x1e\x1e\x8c\xda\x01\x02\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x41\x01\x01\x0d\x00\x00\x00\x09\x00\x00\x00\x61\x6e\x6f\x6e\x79\x6d\x6f\x75\x73\xff\xff\xff\xff\xff\xff\xff\xff"
CLOSE_SESSION_MSG = b"\x4d\x53\x47\x46\x3f\x00\x00\x00\x30\x80\xb7\x75\x4e\x01\xc2\xe7\x05\x00\x00\x00\x05\x00\x00\x00\x01\x00\xd9\x01\x02\x00\x00\xc4\xcc\x27\x76\x44\x73\x45\x91\xaf\x8c\xda\x01\x04\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\x00"
CLOSE_CHANNEL_MSG = b"\x43\x4c\x4f\x46\x39\x00\x00\x00\x30\x80\xb7\x75\x4e\x01\xc2\xe7\x06\x00\x00\x00\x06\x00\x00\x00\x01\x00\xc4\x01\x00\x00\x4b\xfa\x60\x91\xaf\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00"
CLOSE_CHANNEL_MSG_2ND_POSITION = b"\x43\x4c\x4f\x46\x39\x00\x00\x00\x30\x80\xb7\x75\x4e\x01\xc2\xe7\x02\x00\x00\x00\x02\x00\x00\x00\x01\x00\xc4\x01\x00\x00\x4b\xfa\x60\x91\xaf\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00"
CLOSE_SESSION_MSG_4TH_POSITION = b"\x4d\x53\x47\x46\x3f\x00\x00\x00\x30\x80\xb7\x75\x4e\x01\xc2\xe7\x04\x00\x00\x00\x04\x00\x00\x00\x01\x00\xd9\x01\x02\x00\x00\xc4\xcc\x27\x76\x44\x73\x45\x91\xaf\x8c\xda\x01\x04\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\x00"

# First memory leak found with a poor gan: Ticket 1434 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1434
BROWSE_MSG_BUG1 = b"\x4d\x53\x47\x46\xa7\x00\x00\x00\xc9\xc8\x47\x2e\x8c\xa7\x95\xf9\x04\x00\x00\x00\x04\x00\x00\x00\x80\x80\x80\x80\x80\x80\x80\xf8\xc3\xf8\xe3\xa4\xce\xb9\x80\x8c\xab\xff\x81\x80\xd2\x80\x80\x80\x80\x80\x80\xff\xff\xff\xff\xf6\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x82\x80\x80\x82\x80\x80\x80\x80\xb7\x80\x80\x80\x81\x80\x80\x80\x81\x80\x97\x82\x80\x80\x80\x80\x80\x85\xae\xfe\x88\x84\x86\x80\x80\x81\x80\x80\xbf\x80\x80\x80\x8b\x80\x81\x80\x80\x81\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\xb4\x80\x80\x80\x80\x80\x80\x80\x88\x80\x80\x80\x80\x88\x80\x80\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80\x92\x80\x80"

# Problem with Id 528: Ticket 1442 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1442
BROWSE_MSG_BUG2 = b"\x4d\x53\x47\x46\xed\x00\x00\x00\xbf\x00\xc8\x67\xa0\xa2\x6d\xae\x04\x00\x00\x00\x04\x00\x00\x00\x01\x00\x10\x02\x02\x00\x00\x1c\x25\x61\xfb\x00\x00\x00\x00\x00\x00\x00\x00\x52\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x14\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe7\x03\x00\x00\x06\x00\x00\x00\x01\x00\xd1\x69\x02\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x3e\x00\x00\x00\x00\x00\x92\x44\x00\x00\x00\x00\x00\x00\x00\xfe\x01\x00\x03\x4a\x00\x00\x00\x08\x05\x08\x14\x01\x01\x01\x69\x49\x72\x85\x73\x83\x86\x8a\x72\x3b\x36\x47\x08\x07\x0f\x29\x0b\x04\x0c\xfa\x09\x05\x05\x3e\x05\x03\x04\x0c\x09\xdd\x5d\x01\x01\x03\x01\x01\x00\x04\xfe\x06\x06\x04\x71\x01\x04\x06\x0f\x02\x54\x61\x01\x03\x00\x13\x04\x08\x0b\xf6\x0b\x07\x02\x4f\x13\x01\x03\x0a\x0a\x1d\x0e\x08\x07\x04\x11\x1b\x1a\x05\x07\x05\x0e\x0d\x0b\x0e\x0f\x09\x07\x0d\x02\x06\x07\x0a\x05\x19\x06\x0a\x0d\x0a\x06\x02\x09\x27\x1d\x11\x05\x0a\x49\x05\x01\x01"

# Problem with TcpInternalError: Ticket 1444 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1444
BROWSE_MSG_BUG3 = b"\x4d\x53\x47\x46\x09\x00\x00\x00\xbf"

# Problem with client trying to connect in an unsecure way over an encrypted channel: Ticket 1449 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1449
BROWSE_MSG_BUG5 = b""

# Parameters for the bug number 4
# 4: Ticket 1450 https://gitlab.com/systerel/S2OPC/-/issues/1450
SEED = 50 # Seed for the generator
SERVICE = 527 # Service to fuzz
BROWSE_SIZE_MAX = 10 # Maximum size of a browse packet
NUMBER_OF_BROWSE_REQUEST = 30 # Number of browse request send to trigger issue https://gitlab.com/systerel/S2OPC/-/issues/1450
FILE_BROWSE_REQUESTS = "/tmp/browse_packets.txt" # File where the browse packets will be recorded

BAD_SECURE_CHANNEL_CLOSE="80860000"
BAD_SECURITY_CHECK_FAILED="80130000"

DIAGNOSTIC_UNEXPECTED_MESSAGE=b"Closing secure channel on reception of unexpected OPC UA message type"
DIAGNOSTIC_UNKNOWN_INVALID_MESSAGE=b"Closing secure channel on reception of unknown or invalid OPC UA message type"
DIAGNOSTIC_EMPTY=None

STATUS_CHECK_RESPONSE_STATUS_MASK=0x1
STATUS_CHECK_RESPONSE_DIAGNOSTIC_MASK=0x2

# single client, Id , Malformed packet, Expect to succeed connection, Check server response, Expected Status code, Expected Diagnostic Information
CONTEXT_SCENARIO=[(True, 1, (BROWSE_MSG_BUG1, True, True, BAD_SECURE_CHANNEL_CLOSE, DIAGNOSTIC_UNKNOWN_INVALID_MESSAGE)),
                  (True, 2, (BROWSE_MSG_BUG2, True, True, BAD_SECURE_CHANNEL_CLOSE, DIAGNOSTIC_UNEXPECTED_MESSAGE)),
                  (True, 3, (BROWSE_MSG_BUG3, True, True, BAD_SECURITY_CHECK_FAILED, DIAGNOSTIC_EMPTY)),
                  (False, 4, (None, True, False, None, None)),
                  (True, 5, (BROWSE_MSG_BUG5, False, False, None, None))]

def get_security_tokens(packet):
    """
    Inputs: "packet", a sequence of bytes, should be the answer of the OpenSecureChannel request

    Return the ChannelID and the TokenID present in the OpenSecureChannel answer.

    Return: two sequences of 4 bytes
    """
    # Position of the tokens after the analysis of the packet from wireshark
    ChannelID = packet[111:115]
    TokenID = packet[115:119]

    return ChannelID, TokenID



def creating_msg_createsession(ChannelID, TokenID):
    """
    Inputs: "ChannelID" and "TokenID", two sequences of 4 bytes each, should be the output of the "get_security_tokens" function

    Modify the CreateSession message in order to introduce the ChannelId and TokenID.

    Return: CreateSession message modified in bytes
    """
    global CREATE_SESSION_MSG
    tmp = bytearray(CREATE_SESSION_MSG)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    return bytes(tmp)



def get_identifier_numeric(packet):
    """
    Inputs: "packet", a sequence of bytes, should be the answer of the CreateSession request

    Return the Identifier Numeric present in the CreateSession request.

    Return: a sequence of 4 bytes
    """

    return packet[62:66]



def creating_msg_activatesession(ChannelID, TokenID, IdentifierNumeric):
    """
    Inputs: "ChannelId" and "TokenID" from the return of "get_security_tokens" function. "IdentifierNumeric" from the return
    of "get_identifier_numeric".

    Modify the ActivateSession message in order to introduce the "ChannelID", "TokenID" and "IdentifierNumeric".

    Return: ActivateSession message modified in bytes
    """
    global ACTIVATE_SESSION_MSG
    tmp = bytearray(ACTIVATE_SESSION_MSG)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    tmp[31:35] = IdentifierNumeric
    return bytes(tmp)



def replay_msg_browse(browse_msg=None, ChannelID=None, TokenID=None, IdentifierNumeric=None):
    """
    Inputs: "browse_msg" bytes, the browse request to replay. "ChannelId" and "TokenID" from the return of "get_security_tokens" function. "IdentifierNumeric" from the return
    of "get_identifier_numeric" function.

    Modify the "browse_msg" message in order to introduce the "ChannelID", "TokenID" and "IdentifierNumeric" if possible.

    Return: bytes
    """

    tmp = bytearray(browse_msg)
    len_tmp = len(tmp)

    # ChannelID
    for i in range(min(4, len_tmp - 8)):
        tmp[8 + i] = ChannelID[i]

    # TokenID
    for i in range(min(4, len_tmp - 12)):
        tmp[12 + i] = TokenID[i]

    # IdentifierNumeric
    for i in range(min(4, len_tmp - 31)):
        tmp[31 + i] = IdentifierNumeric[i]

    return bytes(tmp)



def random_msg_browse():
    """
    Create a browse message with random length and random values.

    Return: bytes
    """

    browse_msg = bytearray()
    len_browse_msg = randint(1, BROWSE_SIZE_MAX)
    for i in range(len_browse_msg):
        browse_msg.append(randint(0, 255))

    # Security Sequence Number
    for i in range(min(4, len_browse_msg - 16)):
        if i == 0:
            browse_msg[16 + i] = 4
        else:
            browse_msg[16 + i] = 0

    # Security RequestId
    for i in range(min(4, len_browse_msg - 20)):
        if i == 0:
            browse_msg[20 + i] = 4
        else:
            browse_msg[20 + i] = 0

    if len_browse_msg >= 1:
        browse_msg[0] = ord('M')
    if len_browse_msg >= 2:
        browse_msg[1] = ord('S')
    if len_browse_msg >= 3:
        browse_msg[2] = ord('G')
    if len_browse_msg >= 4:
        browse_msg[3] = ord('F')

    # NodeId EncodingMask
    if len_browse_msg >= 25:
        browse_msg[24] = 1

    # NodeId Identifier Numeric
    if len_browse_msg >=28:
        browse_msg[26:28] = int(SERVICE).to_bytes(2, 'little')

    # Message Size
    len_browse_msg_bytes = int(len_browse_msg).to_bytes(4, 'little')
    for i in range(min(4, len_browse_msg - 4)):
        browse_msg[4 + i] = len_browse_msg_bytes[i]

    return browse_msg


def creating_msg_closesession(baseMessage, ChannelID, TokenID, IdentifierNumeric):
    """
    Inputs: "ChannelId" and "TokenID" from the return of "get_security_tokens" function. "IdentifierNumeric" from the return
    of "get_identifier_numeric"

    Modify the CloseSession message in order to introduce the ChannelID, TokenID and IdentifierNumeric

    Return: CloseSession message modified in bytes
    """
    tmp = bytearray(baseMessage)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    tmp[31:35] = IdentifierNumeric
    return bytes(tmp)



def creating_msg_closechannel(baseMessage, ChannelID, TokenID):
    """
    Inputs: "ChannelId" and "TokenID" from the return of "get_security_tokens" function.

    Modify the CloseSecureChannel message in order to introduce the ChannelID and TokenID

    Return: CloseSecureChannel modified in bytes
    """
    tmp = bytearray(baseMessage)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    return bytes(tmp)

def establish_channel():
    status = 1
    ChannelID, TokenID = None , None
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((HOST, PORT))
        # Hello request and answer
        sock.sendall(HELLO_MSG)
        hello_answer = sock.recv(1024)
        # OpenSecureChannel request and answer
        sock.sendall(OPEN_SECURE_CHANNEL_MSG)
        open_secure_answer = sock.recv(1024)
        ChannelID, TokenID = get_security_tokens(open_secure_answer)
        status = 0
    except Exception as e:
        sock.close()
        signal.alarm(0)
        raise e

    return (status, ChannelID, TokenID, sock)

def establish_connection():
    """
    Output: (status, (ChannelID, TokenID, IdentifierNumeric, socket))

    Establish connection with the server. return the ChannelID, TokenID and IdentifierNumeric to send faulty packet and close properly the session.
    A status is also returned, 0 in case of successful connection and session activation, 100 in case of timeout and 1 in case of exception.
    """
    status, ChannelID, TokenID, sock = 1, None , None, None
    try:
        # Establish channel
        (status, ChannelID, TokenID, sock) = establish_channel()

        # CreateSession request and answer
        sock.sendall(creating_msg_createsession(ChannelID, TokenID))
        session_answer = sock.recv(4096) # The answer is more or less 3000 bytes

        # ActivateSession
        IdentifierNumeric = get_identifier_numeric(session_answer)
        sock.sendall(creating_msg_activatesession(ChannelID, TokenID, IdentifierNumeric))
        activate_answer = sock.recv(1024)

    except TimeoutError as browse_err:
        status = 100
        if sock is not None:
            sock.close()

    # The connection has been reset by the server
    except ConnectionResetError as reset:
        pass

    # Problem with the connection
    except Exception as e:
        if sock is not None:
            sock.close()
        signal.alarm(0)
        raise e

    return (status, (ChannelID, TokenID, IdentifierNumeric, sock))

def send_request(browse_msg, channelContext):
    """
    Inputs: "browse_msg" a browse request to replay in a form of bytes. "channelContext" a tuple with connection information

    Send "browse_msg" to a connected socket. Return "status", status take following values : 0 if connection succeed, 100 in case of a timeout, 1 in case of an exception.
    """

    # After the timeout, raise a "TimeoutError" exception to prevent an infinite wait while waiting for send message
    status = 0
    ChannelID, TokenID, IdentifierNumeric, sock = channelContext
    try:
        # Browse request
        browse_request = replay_msg_browse(browse_msg, ChannelID, TokenID, IdentifierNumeric)
        sock.sendall(browse_request)

    # The browse request didn't go
    except TimeoutError as browse_err:
        status = 100

    # The connection has been reset by the server
    except ConnectionResetError as reset:
        pass

    # Problem with the connection
    except Exception as e:
        status = 1
        browse_answer = None
        signal.alarm(0)
        raise e

    return status

def wait_response(channelContext):
    """
    Outputs: (status, server_response)

    Wait for server response and close the channel, retrieve status and response in form of a bytearray.
    """
    ChannelID, TokenID, IdentifierNumeric, sock = channelContext
    status = 0
    browse_answer = None
    try :
        browse_answer = sock.recv(4096)
        browse_answer = bytearray(browse_answer)

        try :
            # CloseSession
            sock.sendall(creating_msg_closesession(CLOSE_SESSION_MSG, ChannelID, TokenID, IdentifierNumeric))
            close_session_answer = sock.recv(1024)

            # CloseChannel
            sock.sendall(creating_msg_closechannel(CLOSE_CHANNEL_MSG, ChannelID, TokenID))

        # If an error message is sent from server it will cause it to close socket connection and lead to broken pipe
        # If there is no err message then close channel
        except BrokenPipeError as e:
            pass
    # The browse response didn't come
    except TimeoutError as _:
        browse_answer = None
        status = 100

    # The connection has been reset by the server
    except ConnectionResetError as reset:
        pass

    # Problem with the connection
    except Exception as e:
        status = 1
        browse_answer = None
        signal.alarm(0)
        raise e
    return (status, browse_answer)

def clean_channel(channelContext):
    channelContext[3].close() # Socket is stored at fourth position in channelContext tupple

def get_status_code(server_response):
    if server_response != None and len(server_response) >= 12 and server_response[0] == ord('E') and server_response[1] == ord('R') and server_response[2] == ord('R'):
        return server_response[8:12]
    return None

def get_diagnostic_info(server_response):
    if server_response != None and len(server_response) >= 16 and server_response[0] == ord('E') and server_response[1] == ord('R') and server_response[2] == ord('R'):
        return server_response[16:]
    return None

def check_response(status_code_received, diagnostic_received, expected_status_code, expected_diagnostic_info):
        """
        Inputs: "server_response" byterarray from server response.

        In case of unexpected server response return -1 otherwise return 0
        """
        # Check if the browse answer contains the "ERR" bytes
        # Use status as a mask first bit status code and second one diagnostic
        status = 0
        if expected_status_code != None and expected_diagnostic_info != None :
            if status_code_received != int(expected_status_code, 16).to_bytes(4, "little") :
                status |= STATUS_CHECK_RESPONSE_STATUS_MASK
            if expected_diagnostic_info != bytes(diagnostic_received) :
                status |= STATUS_CHECK_RESPONSE_DIAGNOSTIC_MASK
        return status

def multiple_client(scenario_context, logger) :
    """
    Sending NUMBER_THREADS random browse requests and recording them inside FILE_BROWSE_REQUESTS.
    Bug corresponding to Ticket 1450: https://gitlab.com/systerel/S2OPC/-/issues/1450
    """
    seed(SEED)

    # List that will record all the browse requests
    l_fuzz_requests = []
    status = 0
    with open(FILE_BROWSE_REQUESTS, '+wb') as file:
        for _ in range(NUMBER_OF_BROWSE_REQUEST):
            browse_msg = random_msg_browse()
            scenario_context=(browse_msg, True, False, None, None)
            l_fuzz_requests.append(browse_msg)
            status = status + single_client(scenario_context, logger)
            # Log the different packets
            file.write(browse_msg)
            file.write(b"\x99\x98\x97\x96") # Putting that at the end of each request thus we can determinate the end of each request inside a file
                                            # Otherwise some bytes could represent an \n preventing the reconstruction of the packets.
    return status

# Handler to interrupt a function after a SIGALARM
def timeout_handler(num, stack):
    raise TimeoutError


def single_client(scenario_context, logger) :
    browse_message, expect_connection, check_server_response, expected_status_code, expected_diagnostic = scenario_context
    # Following command shall succeed in TIMEOUT timing
    signal.alarm(TIMEOUT)
    (status, channelContext) = establish_connection()
    if not expect_connection :
        res = (status == 100)
        logger.add_test("Establish Connection failed (expected)", res)
        if res:
            status = 0
        else:
            status = 1
    else :
        logger.add_test("Establish Connection succeed", status == 0)
        status = send_request(browse_message, channelContext)
        logger.add_test("Send faulty packet succeed", status == 0)
        status, server_response = wait_response(channelContext)
        clean_channel(channelContext)
        if check_server_response:
            logger.add_test("Response received from server", status == 0)
            received_status_code = get_status_code(server_response)
            received_diagnostic_info = get_diagnostic_info(server_response)
            status = check_response(received_status_code, received_diagnostic_info, expected_status_code, expected_diagnostic)
            logger.add_test(f"""Expected status code from server match. expected {expected_status_code}
                            received {hex(int.from_bytes(received_status_code, 'little'))
                            if received_status_code != None else None}""", not(status & STATUS_CHECK_RESPONSE_STATUS_MASK))
            logger.add_test(f"""Expected diagnostic from server match. expected {expected_diagnostic}
                            received {bytes(received_diagnostic_info) if received_diagnostic_info != None else None}""",
                            not(status & STATUS_CHECK_RESPONSE_DIAGNOSTIC_MASK))
        else:
            # ignore the server response status
            status = 0
    signal.alarm(0)
    return status

###############################################################################
# Audit event test functions and declarations
###############################################################################

# Audit log messages.
AUDIT_FAIL_CLO = r'AUDIT\s*{\s*"0:ReceiveTime":".*?",\s*"0:ClientAuditEntryId":".*?",\s*"0:SourceNode":"i=2253",\s*"0:Status":"false",\s*"0:Time":".*?",\s*'\
                 r'"0:SourceName":"SecureChannel/CloseSecureChannel",\s*"0:EventId":".*?",\s*"0:ActionTimeStamp":".*?",\s*'\
                 r'"0:LocalTime":"<ExtensionObject>",\s*"0:Severity":"10",\s*"0:EventType":"i=2059",\s*'\
                 r'"0:Message":"[^"]*? Failure .*?'

AUDIT_FAIL_CLOSE_SESSION = r'AUDIT\s*{\s*"0:ReceiveTime":".*?",\s*"0:ClientAuditEntryId":".*?",\s*"0:SourceNode":"i=2253",\s*"0:Status":"false",\s*'\
                           r'"0:EventId":".*?",\s*"0:SessionId":"i=0",\s*"0:SourceName":"Session/CloseSession",\s*"0:Time":".*?",\s*"0:ActionTimeStamp":".*?",\s*'\
                           r'"0:LocalTime":"<ExtensionObject>",\s*"0:Severity":"10",\s*"0:EventType":"i=2069",\s*'\
                           r'"0:Message":"[^"]*? Session with invalid id .*'

def run_scenario(scenario, logger) :
    signal.alarm(TIMEOUT)
    logger.begin_section(f"Faulty packet audit scenario {scenario}")
    sock = None
    status = 1
    # 1. Send CLO BAD channelId.
    if scenario == 1:
        print("Scenario 1 playing.")
        (status, ChannelID, TokenID, sock) = establish_channel()
        logger.add_test("Establish Connection success", status == 0)
        if status == 0:
            try:
                # CLO request and answer
                ChannelID = b"\x00\x00\x00\x00"
                sock.sendall(creating_msg_closechannel(CLOSE_CHANNEL_MSG_2ND_POSITION, ChannelID, TokenID))
                clo_answer = sock.recv(1024)
            except socket.error as e:
                status = 1
                raise

    # 2. OPN, CreateSession, ActivateSession, CloseSession BAD.
    elif scenario == 2:
        print("Scenario 2 playing.")
        (status, ChannelID, TokenID, sock) = establish_channel()
        logger.add_test("Establish Connection success", status == 0)
        if status == 0:
            try:
                # CreateSession request and answer
                sock.sendall(creating_msg_createsession(ChannelID, TokenID))
                session_answer = sock.recv(4096) # The answer is more or less 3000 bytes
                # ActivateSession
                IdentifierNumeric = get_identifier_numeric(session_answer)
                sock.sendall(creating_msg_activatesession(ChannelID, TokenID, IdentifierNumeric))
                activate_answer = sock.recv(1024)
                # Bad CloseSession: get bad identifier, the one containing the sessionId is the response of CreateSession.
                BadIdentifierNumeric = b"\x00\x00\x00\x00"
                sock.sendall(creating_msg_closesession(CLOSE_SESSION_MSG_4TH_POSITION, ChannelID, TokenID, BadIdentifierNumeric))
                close_session_answer = sock.recv(1024)
                status = 0
            except socket.error as e:
                status = 1
                raise
    else:
        print("Invalid scenario")
        return 1

    sock.close()
    signal.alarm(0)
    return status

def parseLog(file, message):
    pattern = re.compile(message)
    with open(file, 'r', encoding='utf-8') as f:
        content = f.read()
        return bool(pattern.search(content))

###############################################################################
# MAIN
###############################################################################

if __name__ == '__main__':
    # Specifying the system to use SIGALARM to stop a function
    signal.signal(signal.SIGALRM, timeout_handler)
    globRes = 0
    parser = argparse.ArgumentParser(description="""Run single or multiple client connected to a server then send a malformed packet""")
    parser.add_argument('--scenario', required=True, type=int, help=f"Scenario which is run can take value from 1 to {len(CONTEXT_SCENARIO)}")
    parser.add_argument('--logFile', required=False, type=str, help=f"Audit log file")
    args = parser.parse_args()

    if ((args.scenario - 1) in range(len(CONTEXT_SCENARIO))):
        # Faulty packet
        logger = TapLogger("faulty_packet_scen_%d.tap" %(args.scenario))

        is_single, id, client_context = CONTEXT_SCENARIO[args.scenario - 1]
        logger.begin_section(f"faulty packet scenario {id}")
        try :
            if is_single:
                res = single_client(client_context, logger)
            else :
                res = multiple_client(client_context, logger)
            if res != 0:
                globRes = 1
        except Exception as e :
            logger.add_test(f"Exception occur {e}", False)
            sys.exit(1)

    elif (args.scenario - 1 == 5):
        if args.logFile is None:
            print("Please specify the audit log file with option --logFile for this test")
            sys.exit(10)
        # Audit test with faulty packets
        log_file = args.logFile
        logger = TapLogger("faulty_packet_test_audit_events.tap")

        res = run_scenario(1, logger)
        logger.add_test("Test success", res == 0)
        if res != 0:
            globRes = 1

        res = parseLog(log_file, AUDIT_FAIL_CLO)
        logger.add_test("Parsed event fail CLO", res == True)
        if not res:
            globRes = 1

        res = run_scenario(2, logger)
        logger.add_test("Test success", res == 0)
        if res != 0:
            globRes = 1

        res = parseLog(log_file, AUDIT_FAIL_CLOSE_SESSION)
        logger.add_test("Parsed event fail CloseSession", res == True)
        if not res:
            globRes = 1

    else:
        # Invalid scenario number
        sys.exit(1)

    logger.finalize_report(globRes != 0)
    sys.exit(globRes)
