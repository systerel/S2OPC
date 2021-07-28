*** Settings ***
Suite Setup		Setup
Suite Teardown	Teardown
Test Teardown	Test Teardown
Resource		${RENODEKEYWORDS}

Library			S2OPCTest.py

*** Test Cases ***

Publisher is Running
	Log To Console		Launching renode server
	Execute Command		path add @/builds/systerel/S2OPC
	Execute Command		path add @${EXECDIR}
	Execute Command		help path
	Execute Script		ci.repl
	Start Emulation

	Log To Console	Waiting 10 seconds to let the publisher start
	Sleep	10s		Letting the Publisher starts

	Log To Console		Testing good publisher start
	Wait Publisher

Good PubSub Communication
	Log To Console		Launching renode server
	Execute Command		path add @/builds/systerel/S2OPC
	Execute Command		path add @${EXECDIR}
	Execute Script		ci.repl
	Start Emulation

	Log To Console	Waiting 10 seconds to let the publisher start
	Sleep	10s		Letting the Publisher starts

	Log To Console 		Testing Static Configuration
	Test Static Configuration
