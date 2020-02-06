# Small makefile for translation and clean

.PHONY: all clean

all:
	g++ -std=gnu++11 -c -fdump-ada-spec -C translate_wrapper.h

clean:
	rm libs2opc_client_h.ads 2> /dev/null || true
	rm stdint_h.ads 2> /dev/null || true
	rm libs2opc_client.s 2> /dev/null || true
	rm translate_wrapper.s 2> /dev/null || true

