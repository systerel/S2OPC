PATHOBJ =./build
ifndef PATHEXEC
    PATHEXEC =./out
endif
PATHGENTRAD=./B_model/tradc
PATHTRAD=./src/genc
PATHIMPL=./src/impl
PATHTEST=./src/test
PATHADDS=./address_space_generation

ifndef STACK_INSTALL
    STACK_INSTALL=../install_stack
endif
STACK_H   =$(STACK_INSTALL)/include
STACK_LIB =$(STACK_INSTALL)/lib
CRYPTO_LIB = ../libraries/mbedtls-2.3.0/library

CC=gcc
C99FLAGS=-std=c99 -pedantic
CFLAGS  =-g3 -c -Wall -Wextra -O0 -Wno-unused-parameter $(C99FLAGS)
LFLAGS  =-g
INCLUDES=-I$(PATHTRAD) -I. -I$(STACK_H) -I$(PATHIMPL) -I$(PATHTEST) -I$(PATHADDS)
LIBS    =-L$(STACK_LIB) -L$(CRYPTO_LIB) -lm -lingopcs -lmbedcrypto -lmbedx509 -lpthread -lrt

OBJS        =toolkit_header_init.o \
             address_space.o address_space_it.o constants.o msg_read_request.o service_read.o service_read_it.o \
             address_space_bs.o constants_bs.o msg_read_request_bs.o msg_read_response_bs.o \
             service_write_decode_bs.o response_write_bs.o \
             service_get_endpoints_bs.o \
             gen_addspace.o util_variant.o \
             testlib_read_response.o wrap_read.o testlib_write.o \
             service_browse.o service_browse_seq.o service_browse_seq_it.o \
             msg_browse_response_bs.o service_browse_decode_bs.o
OBJS        += toolkit_header.o io_dispatch_mgr.o service_mgr.o session_mgr.o session_core.o session_core_1_bs.o \
               session_async_bs.o session_request_handle_bs.o \
               channel_mgr_bs.o request_handle_bs.o message_in_bs.o message_out_bs.o \
               service_response_cli_cb_bs.o \
               session_core_channel_lost_it_bs.o session_core_orphaned_it_bs.o \
               util_b2c.o test_results.o util_discovery_services.o
OBJS        += sopc_services_events.o sopc_toolkit_config.o
CLIENT_OBJS =toolkit_test_client.o
SERVER_OBJS =toolkit_test_server.o
READ_OBJS   =toolkit_test_read.o
WRITE_OBJS  =toolkit_test_write.o

CLIENT_EXEC :=$(patsubst %.o,$(PATHEXEC)/%,$(CLIENT_OBJS))
SERVER_EXEC :=$(patsubst %.o,$(PATHEXEC)/%,$(SERVER_OBJS))
READ_EXEC   :=$(patsubst %.o,$(PATHEXEC)/%,$(READ_OBJS))
WRITE_EXEC  :=$(patsubst %.o,$(PATHEXEC)/%,$(WRITE_OBJS))
SERVER_OBJS += add.o config_toolkit_server.o
OBJS        :=$(patsubst %,$(PATHOBJ)/%,$(OBJS))
CLIENT_OBJS :=$(patsubst %,$(PATHOBJ)/%,$(CLIENT_OBJS))
SERVER_OBJS :=$(patsubst %,$(PATHOBJ)/%,$(SERVER_OBJS))
READ_OBJS   :=$(patsubst %,$(PATHOBJ)/%,$(READ_OBJS))
WRITE_OBJS  :=$(patsubst %,$(PATHOBJ)/%,$(WRITE_OBJS))

.PHONY: all config clean trad client_server_test

default: all

all: config $(OBJS) $(CLIENT_EXEC) $(SERVER_EXEC) $(READ_EXEC) $(WRITE_EXEC)

config:
	@echo "Configuring Toolkit build and output directory..."
	@mkdir -p $(PATHOBJ)
	@mkdir -p $(PATHEXEC)

clean:
	@echo "Cleaning Toolkit..."
	@# Protect against executing rm -r /*
	@[ -d "$(PATHOBJ)" ] && \rm -rf $(PATHOBJ) || true
	@[ -d "$(PATHEXEC)" ] && \rm -rf $(PATHEXEC) || true

trad:
	@m -s | grep TOTAL | awk -F '|' '{ err = $$3 ~ "OK" && $$10 ~ "OK" ? 0:1 } END {if(err == 1) print "Please typecheck and B0check before make trad"; exit err}'
	@\rm -fr ./$(PATHTRAD)
	@\rm -fr ./$(PATHGENTRAD)
	m -tr
	@mkdir $(PATHTRAD)
	@cp $(PATHGENTRAD)/*.c $(PATHGENTRAD)/*.h $(PATHTRAD)
	@# Renames unwanted files, such as the ones that define a void * to a t_entier4
	@for i in constants_bs.h; do \
	    echo "mv $(PATHTRAD)/$$i $(PATHTRAD)/$$i""_"; \
	    mv $(PATHTRAD)/"$$i" $(PATHTRAD)/"$$i"_; \
	done

$(PATHOBJ)/%.o: $(PATHTRAD)/%.c
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(PATHOBJ)/%.o: $(PATHIMPL)/%.c
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(PATHOBJ)/%.o: $(PATHTEST)/%.c
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(PATHOBJ)/%.o: $(PATHADDS)/%.c
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(CLIENT_EXEC): config $(OBJS) $(CLIENT_OBJS)
	@echo "Linking $@"
	@$(CC) $(OBJS) $(CLIENT_OBJS) $(LIBS) $(LFLAGS) -o $(CLIENT_EXEC)

$(SERVER_EXEC): config $(OBJS) $(SERVER_OBJS)
	@echo "Linking $@"
	@$(CC) $(OBJS) $(SERVER_OBJS) $(LIBS) $(LFLAGS) -o $(SERVER_EXEC)

$(READ_EXEC): config $(OBJS) $(READ_OBJS)
	@echo "Linking $@"
	@$(CC) $(OBJS) $(READ_OBJS) $(LIBS) $(LFLAGS) -o $(READ_EXEC)

$(WRITE_EXEC): config $(OBJS) $(WRITE_OBJS)
	@echo "Linking $@"
	@$(CC) $(OBJS) $(WRITE_OBJS) $(LIBS) $(LFLAGS) -o $(WRITE_EXEC)

toolkit_test_read: $(READ_EXEC)
	@echo "Executing Toolkit read test"
	./run_read_test.sh $(PATHEXEC)

toolkit_test_write: $(WRITE_EXEC)
	@echo "Executing Toolkit write test"
	./run_write_test.sh $(PATHEXEC)

client_server_test: $(CLIENT_EXEC) $(SERVER_EXEC)
	@echo "Executing Toolkit client / server test"
	./run_client_server_test.sh $(PATHEXEC)


