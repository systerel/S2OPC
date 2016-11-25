#!/bin/bash

# -keyform der does not work...
openssl req -x509 -newkey rsa:1024 -nodes -keyout test_suite.key.pem -out test_suite.der -outform der -days 100
openssl pkey -in test_suite.key.pem -out test_suite.key -outform der
echo "Signed public key:"
hexdump -ve '/1 "%02x"' test_suite.der
echo -e "\nPrivate key"
hexdump -ve '/1 "%02x"' test_suite.key
