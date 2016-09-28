# OS DEPENDENCIES
OSTYPE=$(shell echo $$OSTYPE)

ifeq ($(OSTYPE),$(filter linux% darwin%,$(OSTYPE)))
     CC=gcc
     EXCLUDE_DIR="*win32*"
     LIBS=$(LIBS_MBEDTLS) -lssl -lcrypto -lrt -lpthread
     COPY_SSL=@cat /dev/null
else
    CC=i686-w64-mingw32-gcc #i686-pc-mingw32-gcc
    EXCLUDE_DIR="*linux*"
    OPEN_SSL_FILES=$(WORKSPACE_DIR)/lib/openssl/include
    INCLUDES_SSL=-I$(OPEN_SSL_FILES)
    LIBS=$(LIBS_MBEDTLS) -lcrypt32 -lrpcrt4 -lws2_32 -llibeay32 -lssleay32
    SSL_DLL_DIR=$(WORKSPACE_DIR)/lib/openssl/bin/vs2008
    SSL_DIR=$(WORKSPACE_DIR)/lib/openssl/lib/win32/
    LIBS_DIR=-L$(SSL_DIR)
    COPY_SSL=cp $(SSL_DLL_DIR)/* ../out/
endif


# OUTPUTS
WORKSPACE_DIR=.
CERT_DIR=$(WORKSPACE_DIR)/cert
EXEC_DIR=$(WORKSPACE_DIR)/out
BUILD_DIR=$(WORKSPACE_DIR)/build
BUILD_DIR_SED=$(subst /,\/,$(BUILD_DIR))

# INPUTS
UASTACK_DIR=$(WORKSPACE_DIR)/src/
STUBSERVER_DIR=$(WORKSPACE_DIR)/stub_server
STUBCLIENT_DIR=$(WORKSPACE_DIR)/stub_client
TESTS_DIR=$(WORKSPACE_DIR)/test/ingopcs
UASTACK_DIRS=$(shell find $(UASTACK_DIR) -not -path $(EXCLUDE_DIR) -type d)
UASTACK_SRCS=$(shell find $(UASTACK_DIR) -not -path $(EXCLUDE_DIR) -type f -name "*.c" ! -name "*.h")
STUBSERVER_SRC=$(shell find $(STUBSERVER_DIR) -type f -name "*.c" ! -name "*.h")
STUBCLIENT_SRC=$(shell find $(STUBCLIENT_DIR) -type f -name "*.c" ! -name "*.h")
UASTACK_SRC_FILES=$(foreach src, $(UASTACK_SRCS), $(shell basename $(src)))
UASTACK_OBJ_FILES=$(patsubst %.c,$(BUILD_DIR)/%.o,$(UASTACK_SRC_FILES))
TESTS_SRC=$(shell find $(TESTS_DIR) -type f -name "*.c" ! -name "*.h")
TESTS_SRC_FILES=$(foreach src, $(TESTS_SRCS), $(shell basename $(src)))

# MBEDTLS INPUTS
MBEDTLS_DIR=$(WORKSPACE_DIR)/lib/mbedtls-2.3.0
INCLUDES_MBEDTLS=-I$(MBEDTLS_DIR)/include
LIBS_MBEDTLS=-L$(MBEDTLS_DIR)/library -lmbedtls -lmbedx509 -lmbedcrypto


# C COMPILER CONFIG
CCFLAGS=-c -g -Wall -Wextra -O0 #-pedantic -std=c99
LFLAGS=-g
INCLUDES=$(INCLUDES_MBEDTLS) $(INCLUDES_SSL) $(addprefix -I, $(UASTACK_DIRS))
DEFS=-DOPCUA_USE_SYNCHRONISATION=0 -DOPCUA_MULTITHREADED=0 -DOPCUA_TRACE_ENABLE=1 #-DOPCUA_HAVE_SERVERAPI=1 -DOPCUA_HAVE_OPENSSL=1

# MAKEFILE CONTENT

.PHONY : all config mbedtls clean clean_mbedtls cleanall
.DELETE_ON_ERROR : .depend

default: all

all: config mbedtls $(EXEC_DIR)/stub_client $(EXEC_DIR)/stub_server #$(EXEC_DIR)/check_stack

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include .depend
endif
endif

config:
	\mkdir -p $(BUILD_DIR) $(EXEC_DIR)
	\mkdir -p $(EXEC_DIR)/revoked $(EXEC_DIR)/untrusted $(EXEC_DIR)/trusted \
	$(EXEC_DIR)/client_private $(EXEC_DIR)/server_private \
	$(EXEC_DIR)/client_public $(EXEC_DIR)/server_public
	\cp $(CERT_DIR)/cacert.der $(EXEC_DIR)/trusted
	\cp $(CERT_DIR)/client.key $(EXEC_DIR)/client_private
	\cp $(CERT_DIR)/client.der $(EXEC_DIR)/client_public
	\cp $(CERT_DIR)/server.key $(EXEC_DIR)/server_private
	\cp $(CERT_DIR)/server.der $(EXEC_DIR)/server_public

$(BUILD_DIR)/%.o:
	$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@ $(DEFS)

.depend: $(UASTACK_SRCS) $(STUBSERVER_SRC) $(STUBCLIENT_SRC) $(TESTS_SRC)
	$(CC) $(CCFLAGS) $(DEFS) $(INCLUDES) -MM $^ > .depend
	sed 's/^\(.*\)\.o:/$(BUILD_DIR_SED)\/\1.o:/g' -i .depend

$(EXEC_DIR)/stub_server: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/stub_server.o
	$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS)
	$(COPY_SSL)

$(EXEC_DIR)/stub_client: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/stub_client.o
	$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS)
	$(COPY_SSL)

$(EXEC_DIR)/check_stack: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/check_stack.o
	$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS) -lcheck -lm

mbedtls:
	$(MAKE) -C $(MBEDTLS_DIR)

check: $(EXEC_DIR)/check_stack
	$(EXEC_DIR)/check_stack

clean_mbedtls:
	$(MAKE) -C $(MBEDTLS_DIR) clean

clean:
	\rm -rf $(BUILD_DIR) $(EXEC_DIR)
	\rm -f .depend

cleanall: clean clean_mbedtls
