PATHOBJ =./out
PATHGENTRAD=./B_model/tradc
PATHTRAD=./src/genc
PATHIMPL=./src/impl
PATHTEST=./src/test
LIB     =libread.a
PATHINST=./install

STACK_H   =../stack/install/include
STACK_LIB =../stack/install/lib
CRYPTO_LIB = ../libraries/mbedtls-2.3.0/library

CFLAGS  =-g -c -Wall -Wextra -O0 -Wno-unused-parameter
LFLAGS  =-g
INCLUDES=-I$(PATHTRAD) -I. -I$(STACK_H) -I$(PATHIMPL) -I$(PATHTEST)
LIBS    =-L$(STACK_LIB) -L$(CRYPTO_LIB) -lingopcs -lmbedcrypto -lmbedx509 -lpthread

OBJS        =address_space.o constants.o msg_read_request.o msg_read_request_it.o msg_read_response.o service_read.o \
             address_space_bs.o constants_bs.o msg_read_request_bs.o msg_read_response_bs.o \
             gen_addspace.o testlib_read_response.o
OBJS        += session_header.o io_dispatch_mgr.o session_mgr.o session_core.o session_core_1_bs.o \
               channel_mgr_bs.o request_handle_bs.o message_in_bs.o message_out_bs.o \
               service_read_cli_cb_bs.o test_results.o \
               session_core_channel_lost_it_bs.o session_core_orphaned_it_bs.o
CLIENT_OBJS =test_sessions.o
SERVER_OBJS =test_server.o
READ_OBJS   =test_read.o

CLIENT_EXEC :=$(patsubst %.o,%,$(CLIENT_OBJS))
SERVER_EXEC :=$(patsubst %.o,%,$(SERVER_OBJS))
READ_EXEC   :=$(patsubst %.o,%,$(READ_OBJS))
OBJS        :=$(patsubst %,$(PATHOBJ)/%,$(OBJS))
CLIENT_OBJS += wrap_read.o
CLIENT_OBJS :=$(patsubst %,$(PATHOBJ)/%,$(CLIENT_OBJS))
SERVER_OBJS += wrap_read.o
SERVER_OBJS :=$(patsubst %,$(PATHOBJ)/%,$(SERVER_OBJS))
READ_OBJS   += wrap_read.o
READ_OBJS   :=$(patsubst %,$(PATHOBJ)/%,$(READ_OBJS))

.PHONY: all config clean trad

default: all

all: config $(OBJS) $(CLIENT_OBJS) $(SERVER_OBJS) $(READ_OBJS)
	gcc $(OBJS) $(CLIENT_OBJS) $(LIBS) $(LFLAGS) -o $(CLIENT_EXEC)
	gcc $(OBJS) $(SERVER_OBJS) $(LIBS) $(LFLAGS) -o $(SERVER_EXEC)
	gcc $(OBJS) $(READ_OBJS)   $(LIBS) $(LFLAGS) -o $(READ_EXEC)
	ar rc $(LIB) $(OBJS)

config:
	@mkdir -p $(PATHOBJ)
	@mkdir -p $(PATHINST)

clean:
	@# Protect against executing rm -r /*
	[ -d "$(PATHINST)" ] && \rm -rf $(PATHINST) || true
	[ -d "$(PATHOBJ)" ] && \rm -rf $(PATHOBJ) || true
	\rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(READ_EXEC) $(LIB)

trad:
	@rm -fr $(PATHTRAD)
	m -tr
	@mkdir $(PATHTRAD)
	@cp $(PATHGENTRAD)/*.c $(PATHGENTRAD)/*.h $(PATHTRAD)
	@# Renames unwanted files, such as the ones that define a void * to a t_entier4
	@for i in constants_bs.h; do \
	    echo "mv $(PATHTRAD)/$$i $(PATHTRAD)/$$i""_"; \
	    mv $(PATHTRAD)/"$$i" $(PATHTRAD)/"$$i"_; \
	done

out/%.o: $(PATHTRAD)/%.c
	gcc $(CFLAGS) $(INCLUDES) $< -o $@

out/%.o: $(PATHIMPL)/%.c
	gcc $(CFLAGS) $(INCLUDES) $< -o $@

out/%.o: $(PATHTEST)/%.c
	gcc $(CFLAGS) $(INCLUDES) $< -o $@

install: all
	@echo "Copying .h and .a to $(PATHINST)"
	@cp ./*.h $(PATHINST)
	@cp $(PATHTRAD)/*.h $(PATHINST)
	@cp $(LIB) $(PATHINST)
