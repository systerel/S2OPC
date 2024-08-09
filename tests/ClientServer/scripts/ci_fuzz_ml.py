import socket
import signal
import sys
from random import randint
from random import seed
import multiprocessing
import time

# Host, port and url of the server running S2OPC
HOST = "127.0.0.1"
PORT = 4841
URL = "opc.tcp://" + HOST + ":" + str(PORT)

# Timeout after which a connection should be closed, useful when no response is given by the server for a browse request
TIMEOUT = 1

# Path to the file containing the potential memory leaks of the OPCUA server
PATH_FILE_MEMORY_LEAKS = "/tmp/toolkit_stderr.txt"

# Messages to setup the OPCUA connection and to close the connection
HELLO_MSG = b"\x48\x45\x4c\x46\x3b\x00\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xfb\xff\x04\x00\x05\x00\x00\x00\x1b\x00\x00\x00\x6f\x70\x63\x2e\x74\x63\x70\x3a\x2f\x2f\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x30\x2e\x31\x3a\x34\x38\x34\x31"
OPEN_SECURE_CHANNEL_MSG = b"\x4f\x50\x4e\x46\x84\x00\x00\x00\x00\x00\x00\x00\x2f\x00\x00\x00\x68\x74\x74\x70\x3a\x2f\x2f\x6f\x70\x63\x66\x6f\x75\x6e\x64\x61\x74\x69\x6f\x6e\x2e\x6f\x72\x67\x2f\x55\x41\x2f\x53\x65\x63\x75\x72\x69\x74\x79\x50\x6f\x6c\x69\x63\x79\x23\x4e\x6f\x6e\x65\xff\xff\xff\xff\xff\xff\xff\xff\x01\x00\x00\x00\x01\x00\x00\x00\x01\x00\xbe\x01\x00\x00\x81\x12\xcf\x1e\x1e\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xff\xff\xff\xff\x60\xea\x00\x00"
CREATE_SESSION_MSG = b"\x4d\x53\x47\x46\xc7\x00\x00\x00\xbf\xab\x84\x6a\x11\x5f\xce\x37\x02\x00\x00\x00\x02\x00\x00\x00\x01\x00\xcd\x01\x00\x00\xc2\x89\xcf\x1e\x1e\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\x13\x00\x00\x00\x75\x72\x6e\x3a\x53\x32\x4f\x50\x43\x3a\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\xff\xff\xff\xff\x02\x10\x00\x00\x00\x53\x32\x4f\x50\x43\x5f\x44\x65\x6d\x6f\x43\x6c\x69\x65\x6e\x74\x01\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\xff\xff\xff\xff\x1b\x00\x00\x00\x6f\x70\x63\x2e\x74\x63\x70\x3a\x2f\x2f\x31\x39\x32\x2e\x31\x36\x38\x2e\x31\x30\x2e\x31\x3a\x34\x38\x34\x31\x13\x00\x00\x00\x53\x32\x4f\x50\x43\x5f\x63\x6c\x69\x65\x6e\x74\x5f\x73\x65\x73\x73\x69\x6f\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x4c\xed\x40\xfb\xff\x04\x00"
ACTIVATE_SESSION_MSG = b"\x4d\x53\x47\x46\x6c\x00\x00\x00\xbf\xab\x84\x6a\x11\x5f\xce\x37\x03\x00\x00\x00\x03\x00\x00\x00\x01\x00\xd3\x01\x02\x00\x00\x3a\x04\x24\x3f\xd9\x2c\xd0\x1e\x1e\x8c\xda\x01\x02\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x41\x01\x01\x0d\x00\x00\x00\x09\x00\x00\x00\x61\x6e\x6f\x6e\x79\x6d\x6f\x75\x73\xff\xff\xff\xff\xff\xff\xff\xff"
CLOSE_SESSION_MSG = b"\x4d\x53\x47\x46\x3f\x00\x00\x00\x30\x80\xb7\x75\x4e\x01\xc2\xe7\x05\x00\x00\x00\x05\x00\x00\x00\x01\x00\xd9\x01\x02\x00\x00\xc4\xcc\x27\x76\x44\x73\x45\x91\xaf\x8c\xda\x01\x04\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x13\x00\x00\x00\x00\x00\x00"
CLOSE_CHANNEL_MSG = b"\x43\x4c\x4f\x46\x39\x00\x00\x00\x30\x80\xb7\x75\x4e\x01\xc2\xe7\x06\x00\x00\x00\x06\x00\x00\x00\x01\x00\xc4\x01\x00\x00\x4b\xfa\x60\x91\xaf\x8c\xda\x01\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00"

# First memory leak found with a poor gan: Ticket 1434 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1434
BROWSE_MSG_BUG1 = b"\x4d\x53\x47\x46\xa7\x00\x00\x00\xc9\xc8\x47\x2e\x8c\xa7\x95\xf9\x04\x00\x00\x00\x04\x00\x00\x00\x80\x80\x80\x80\x80\x80\x80\xf8\xc3\xf8\xe3\xa4\xce\xb9\x80\x8c\xab\xff\x81\x80\xd2\x80\x80\x80\x80\x80\x80\xff\xff\xff\xff\xf6\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x82\x80\x80\x82\x80\x80\x80\x80\xb7\x80\x80\x80\x81\x80\x80\x80\x81\x80\x97\x82\x80\x80\x80\x80\x80\x85\xae\xfe\x88\x84\x86\x80\x80\x81\x80\x80\xbf\x80\x80\x80\x8b\x80\x81\x80\x80\x81\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\xb4\x80\x80\x80\x80\x80\x80\x80\x88\x80\x80\x80\x80\x88\x80\x80\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80\x92\x80\x80"

# Problem with Id 528: Ticket 1442 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1442 
BROWSE_MSG_BUG2 = b"\x4d\x53\x47\x46\xed\x00\x00\x00\xbf\x00\xc8\x67\xa0\xa2\x6d\xae\x04\x00\x00\x00\x04\x00\x00\x00\x01\x00\x10\x02\x02\x00\x00\x1c\x25\x61\xfb\x00\x00\x00\x00\x00\x00\x00\x00\x52\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\x88\x14\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe7\x03\x00\x00\x06\x00\x00\x00\x01\x00\xd1\x69\x02\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x3e\x00\x00\x00\x00\x00\x92\x44\x00\x00\x00\x00\x00\x00\x00\xfe\x01\x00\x03\x4a\x00\x00\x00\x08\x05\x08\x14\x01\x01\x01\x69\x49\x72\x85\x73\x83\x86\x8a\x72\x3b\x36\x47\x08\x07\x0f\x29\x0b\x04\x0c\xfa\x09\x05\x05\x3e\x05\x03\x04\x0c\x09\xdd\x5d\x01\x01\x03\x01\x01\x00\x04\xfe\x06\x06\x04\x71\x01\x04\x06\x0f\x02\x54\x61\x01\x03\x00\x13\x04\x08\x0b\xf6\x0b\x07\x02\x4f\x13\x01\x03\x0a\x0a\x1d\x0e\x08\x07\x04\x11\x1b\x1a\x05\x07\x05\x0e\x0d\x0b\x0e\x0f\x09\x07\x0d\x02\x06\x07\x0a\x05\x19\x06\x0a\x0d\x0a\x06\x02\x09\x27\x1d\x11\x05\x0a\x49\x05\x01\x01"

# Problem with TcpInternalError: Ticket 1444 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1444
BROWSE_MSG_BUG3 = b"\x4d\x53\x47\x46\x09\x00\x00\x00\xbf"

# Problem with client trying to connect in an unsecure way over an encrypted channel: Ticket 1449 gitlab, https://gitlab.com/systerel/S2OPC/-/issues/1449
BROWSE_MSG_BUG5 = b""

# Parameters for the bug number 4
SEED = 50 # Seed for the generator
SERVICE = 527 # Service to fuzz
BROWSE_SIZE_MAX = 10 # Maximum size of a browse packet
NUMBER_THREADS = 30 # Number of threads fuzzing at the same time
FILE_BROWSE_REQUESTS = "tests/ClientServer/scripts/browse_packets.txt" # File where the browse packets will be recorded



def return_security_tokens(packet):
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
    Inputs: "ChannelID" and "TokenID", two sequences of 4 bytes each, should be the output of the "return_security_tokens" function

    Modify the CreateSession message in order to introduce the ChannelId and TokenID.

    Return: CreateSession message modified in bytes
    """
    global CREATE_SESSION_MSG
    tmp = bytearray(CREATE_SESSION_MSG)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    return bytes(tmp)



def return_identifier_numeric(packet):
    """
    Inputs: "packet", a sequence of bytes, should be the answer of the CreateSession request

    Return the Identifier Numeric present in the CreateSession request.

    Return: a sequence of 4 bytes
    """
     
    return packet[62:66]



def creating_msg_activatesession(ChannelID, TokenID, IdentifierNumeric):
    """
    Inputs: "ChannelId" and "TokenID" from the return of "return_security_tokens" function. "IdentifierNumeric" from the return
    of "return_identifier_numeric".

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
    Inputs: "browse_msg" bytes, the browse request to replay. "ChannelId" and "TokenID" from the return of "return_security_tokens" function. "IdentifierNumeric" from the return
    of "return_identifier_numeric" function.

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

    # M
    if len_browse_msg >= 1:
        browse_msg[0] = 77

        # S 
        if len_browse_msg >= 2:
            browse_msg[1] = 83

            # G
            if len_browse_msg >= 3:
                browse_msg[2] = 71

                # F
                if len_browse_msg >= 4:
                    browse_msg[3] = 70

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

    

def creating_msg_closesession(ChannelID, TokenID, IdentifierNumeric):
    """
    Inputs: "ChannelId" and "TokenID" from the return of "return_security_tokens" function. "IdentifierNumeric" from the return
    of "return_identifier_numeric"

    Modify the CloseSession message in order to introduce the ChannelID, TokenID and IdentifierNumeric

    Return: CloseSession message modified in bytes
    """
    global CLOSE_SESSION_MSG
    tmp = bytearray(CLOSE_SESSION_MSG)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    tmp[31:35] = IdentifierNumeric
    return bytes(tmp)



def creating_msg_closechannel(ChannelID, TokenID):
    """
    Inputs: "ChannelId" and "TokenID" from the return of "return_security_tokens" function.

    Modify the CloseSecureChannel message in order to introduce the ChannelID and TokenID

    Return: CloseSecureChannel modified in bytes
    """
    global CLOSE_CHANNEL_MSG
    tmp = bytearray(CLOSE_CHANNEL_MSG)
    tmp[8:12] = ChannelID
    tmp[12:16] = TokenID
    return bytes(tmp)



def send_request(browse_msg, status, bug_number):
    """
    Inputs: "browse_msg" a browse request to replay in a form of bytes. "status" integer list of length 1. "bug_number" integer.

    Send "browse_msg". Depending on the answer of the server status will change. 0 everything is fine, 1 problem when connecting
    to the server. 2 invalid message sent by the server in response to a browse request: Ticket 1442. 3 invalid message sent
    by the server in response to a browse request: Ticket 1444. 100 no response given from the server after a browse request.
    """

    # After the timeout, raise a "TimeoutError" exception to prevent an infinite wait while waiting for a browse response 
    signal.alarm(TIMEOUT)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            received = False # Variable to check if a browse response has arrived or no

            s.connect((HOST, PORT))

            # Hello request and answer
            s.sendall(HELLO_MSG)
            hello_answer = s.recv(1024)

            # OpenSecureChannel request and answer
            s.sendall(OPEN_SECURE_CHANNEL_MSG)
            open_secure_answer = s.recv(1024)
            ChannelID, TokenID = return_security_tokens(open_secure_answer)

            # CreateSession request and answer
            s.sendall(creating_msg_createsession(ChannelID, TokenID))
            session_answer = s.recv(4096) # The answer is more or less 3000 bytes

            # ActivateSession
            IdentifierNumeric = return_identifier_numeric(session_answer)
            s.sendall(creating_msg_activatesession(ChannelID, TokenID, IdentifierNumeric))
            activate_answer = s.recv(1024)

            # Browse request and answer
            browse_request = replay_msg_browse(browse_msg, ChannelID, TokenID, IdentifierNumeric)
            s.sendall(browse_request)
            browse_answer = s.recv(4096)
            received = True # Browse response received
            browse_answer = bytearray(browse_answer)

        # The browse response didn't come
        except TimeoutError as browse_err:
            status[0] = 100
        
        # The connection has been reset by the server
        except ConnectionResetError as reset:
            pass
            
        # Problem with the connection
        except Exception as e:
            status[0] = 1
            s.close()
            signal.alarm(0)
            raise e

        # Check if the browse answer contains the "ERR" bytes, with E being 69 and R being 82.
        # In that case the connection is closed, thus no messages for closing the session should be sent.
        if received and len(browse_answer) >= 3 and browse_answer[0] == 69 and browse_answer[1] == 82 and browse_answer[2] == 82:
            # Bug related to ticket 1442
            if bug_number == 2 and bytes(browse_answer[16:]) == b"Closing secure channel due to maximum reached (last attempt or oldest without session)":
                status[0] = 2
            # Bug related to ticket 1444
            elif bug_number == 3 and bytes(browse_answer[8:12]) == int("80820000", 16).to_bytes(4, "little"):
                status[0] = 3
        else:
            try:
                # CloseSession
                s.sendall(creating_msg_closesession(ChannelID, TokenID, IdentifierNumeric))
                close_session_answer = s.recv(1024)

                # CloseChannel
                s.sendall(creating_msg_closechannel(ChannelID, TokenID))
                close_channel_answer = s.recv(1024)

            except TimeoutError as browse_err:
                status[0] = 100

            # The connection has been reset by the server
            except ConnectionResetError as reset:
                pass

            except Exception as e:
                status[0] = 1
                s.close()
                signal.alarm(0)
                raise e
    
    signal.alarm(0)



def bug4():
    """
    Sending NUMBER_THREADS random browse requests and recording it inside FILE_BROWSE_REQUESTS.
    Bug corresponding to Ticket 1450: https://gitlab.com/systerel/S2OPC/-/issues/1450
    """
    seed(SEED)

    # List that will record all the browse requests
    l_fuzz_requests = []

    threads = list()
    for index in range(NUMBER_THREADS):
        browse_msg = random_msg_browse()
        l_fuzz_requests.append(browse_msg)
        x = multiprocessing.Process(target=send_request, args=(browse_msg, [0], 4))
        threads.append(x)
        x.start()

    # Wait 2 seconds, thus the threads have time to start
    time.sleep(2)
    for index, thread in enumerate(threads):
        thread.terminate() # Kill the thread
        thread.join()

    # Log the different packets
    with open(FILE_BROWSE_REQUESTS, 'wb') as file:
        for i in l_fuzz_requests:
            file.write(i)
            file.write(b"\x99\x98\x97\x96") # Putting that at the end of each request thus we can determinate the end of each request inside a file
                                            # Otherwise some bytes could represent an \n preventing the reconstruction of the packets. 




# Handler to interrupt a function after a SIGALARM
def timeout_handler(num, stack):
    raise TimeoutError



###############################################################################
# MAIN
###############################################################################

# Specifying the system to use SIGALARM to stop a function
signal.signal(signal.SIGALRM, timeout_handler)

# Status of the request
status = [0] 

try:
    bug_number = int(sys.argv[1])
    if bug_number == 1:
        print("TRYING PACKET: ", BROWSE_MSG_BUG1)
    elif bug_number == 2:
        print("TRYING PACKET: ", BROWSE_MSG_BUG2)
    elif bug_number == 3:
        print("TRYING PACKET: ", BROWSE_MSG_BUG3)
    elif bug_number == 4:
        print("TRYING MULTIPLE PACKETS")
    else:
        print("TRYING PACKET: ", BROWSE_MSG_BUG5)


    if bug_number == 1:
        send_request(BROWSE_MSG_BUG1, status, bug_number)

    elif bug_number == 2:
        send_request(BROWSE_MSG_BUG2, status, bug_number)
        if status[0] == 2:
            print("\n\nInvalid response from the server, see https://gitlab.com/systerel/S2OPC/-/issues/1442")
            sys.exit(2)

    elif bug_number == 3:
        send_request(BROWSE_MSG_BUG3, status, bug_number)
        if status[0] == 3:
            print("\n\nInvalid response from the server, see https://gitlab.com/systerel/S2OPC/-/issues/1444")
            sys.exit(3)

    elif bug_number == 4:
        bug4()

    else:
        send_request(BROWSE_MSG_BUG5, status, bug_number)    

except Exception as e:
    print(e)
    sys.exit(1)
    

