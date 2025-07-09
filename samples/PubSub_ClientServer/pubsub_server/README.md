## PubSub server

This sample application showcases a PubSub application using several key OPC UA functionalities implemented together: it acts as **Publisher** & **Subscriber** using a **Server** to control lifecycle and store the variables data, it also acts as a **Client** to retrieve security keys.

Thus it also demonstrates interaction with a server implementing **Security Key Service (SKS)** to securely exchange keys for PubSub encryption.

# Usage

## Start an SKS server to provide the security keys for security group id "1"
`TEST_PASSWORD_PRIVATE_KEY=password ./test_server_sks 1`

## Launch the sample PubSub server
`TEST_PASSWORD_PRIVATE_KEY=password TEST_PASSWORD_USER_SECUADMIN=1234 ./pubsub_server`

The client will connect to the SKS server to retrieve and distribute keys for Pub/Sub message encryption and decryption.
One optional input argument may be provided: the URL of the SKS server. Default is set to "opc.tcp://localhost:4843"

# Description
This sample combines multiple OPC UA functionalities:
- a Publisher and a Subscriber:
    - Configuration and lifecycle (start/stop) are controlled through nodes in the address space:
      - value of node "ns=1;s=PubSubConfiguration" is loaded as XML configuration file (default is `config_pubsub_server.xml`)
      - starting / stopping PubSub functionnality depending on "ns=1;s=PubSubStartStop" variable value
    - Event-based behavior is controlled through method call in the address space:
      - "ns=1;s=AcyclicSend": send publisher message in acyclic mode
      - "ns=1;s=DataSetMessageFiltering": enable or disable the emission of a certain DataSetMessage defined in configuration.
- a Server: 
  - Loads address space from XML (s2opc_pubsub_nodeset.xml) or statically (if the option PUBSUB_STATIC_CONFIG is set)
  - Variables of folder node PublisherVars ("ns=1;s=PubVars") contain the value of the published variables (used by Publisher)
  - Variables of folder node SubscriberVars ("ns=1;s=SubVars") contain the value of the subscribed variables (updated by Subscriber)
  - Variables "ns=1;s=PubSubConfiguration", "ns=1;s=PubSubStartStop" and "ns=1;s=PubSubStatus" manages lifcycle and configuration of PubSub part:
    - start / stop the Publisher and Subscriber by writing its value: 1 for starting and 0 for stopping
    - obtain status by reading its value: Disabled = 0, Paused = 1, Operational = 2, Error = 3
    - change configuration by writting its value: expected content is the XML configuration file (default is `config_pubsub_server.xml`)
  - Implements methods used to trigger PubSub behavior using Publisher object node ("ns=1;s=Publisher") 
    - "ns=1;s=AcyclicSend": call this method to allow the Publisher to send message in acyclic mode
    - "ns=1;s=DataSetMessageFiltering": call this method to allow Publisher to enable or disable the emission of a certain DataSetMessage.
- a Client: 
  - Hardcoded configuration (no XML)
  - Used by the SK module (Provider) to call the GetSecurityKeys method on the SKS server, it retrieves security keys and provides them to the 
    Publisher and Subscriber
