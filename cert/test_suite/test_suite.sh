#!/bin/bash

# -keyform der does not work...
openssl req -x509 -newkey rsa:2048 -nodes -keyout test_suite.key.pem -out test_suite.der -outform der -days 100
openssl pkey -in test_suite.key.pem -out test_suite.key -outform der
