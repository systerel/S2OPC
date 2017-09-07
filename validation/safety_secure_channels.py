
from opcua import ua
from opcua.crypto import security_policies 

def safety_secure_channels_test(client):
  #client.set_security_string("Basic256,Sign,trusted/cacert.der,client_private/client.key")
  client.set_security(security_policies.SecurityPolicyBasic256,'../bin/trusted/cacert.der','../bin/client_private/client.pem','../bin/server_public/server.der',ua.MessageSecurityMode.Sign)
  client.connect()

