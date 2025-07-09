## PubSub Controller / Device Samples

These two sample applications demonstrate a complete OPC UA PubSub system using SKS **push model** via the `SetSecurityKeys` method.  

Both the **Controller** and the **Device** applications act as **Publisher** & **Subscriber** application configured through XML configuration file and are using a **Server** to store the exchanged variables data. Unlike the `pubsub_server` sample the lifecycle is not managed using the Server address space.

# Usage

Start the device: 
`TEST_PASSWORD_PRIVATE_KEY=password ./device_sks_server_push`

Start the controller:
`TEST_PASSWORD_PRIVATE_KEY=password TEST_PASSWORD_USER_SECUADMIN=1234 ./controller_sks_client`

The two samples will communicate using 2 security group keys and it is possible to update variable in PublisherVars of one server to see result in SubscriberVars of the peer server (cf. XML Pub/Sub configuration to identify which variables are availables).

# Description
Controller sample:
  - Pub/Sub:
    - behavior fully defined in XML (`controller_pubsub_SKS_client.xml`). The XML defines two SecurityGroups, one for the Publisher and one
      for the Subscriber.
  - Server:
    - Configuration and address space loaded via XML files
    - Variables of folder node PublisherVars ("ns=1;s=PubVars") contain the value of the published variables (used by Publisher)
    - Variables of folder node SubscriberVars ("ns=1;s=SubVars") contain the value of the subscribed variables (updated by Subscriber)
  - SKS instance implemented by SK modules that generates random keys used locally for the Pub/Sub instances and push those keys to Device sample using SKS push model. It also manages the keys lifetime (token increase)
  - Client:
    - Configuration via XML file (connection to Device server)
    - Connects and calls SetSecurityKeys method on the Device to distribute keys when a key update occurs in SK modules

Device sample:
  - Pub/Sub:
    - behavior fully defined in XML (`device_pubsub_SKS_push_server.xml`). It is symmetric to the Controller Pub/Sub configuration in order to communicate with it.
  - Server:
    - Configuration and address space loaded via XML files
    - Variables of folder node PublisherVars ("ns=1;s=PubVars") contain the value of the published variables (used by Publisher)
    - Variables of folder node SubscriberVars ("ns=1;s=SubVars") contain the value of the subscribed variables (updated by Subscriber)
    - Implements SetSecurityKeys method (acts as server with SKS push model)
  - SKS instance implemented by SK modules that stores the keys pushed through SetSecurityKeys and that will be used by Pub/Sub to communicate. It also manages the keys lifetime (token increase)

