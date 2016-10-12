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

## Directories
### srcs directories definition
UASTACK_DIR=$(WORKSPACE_DIR)/src/
STUBSERVER_DIR=$(WORKSPACE_DIR)/stub_server
STUBCLIENT_DIR=$(WORKSPACE_DIR)/stub_client
TESTS_DIR=$(WORKSPACE_DIR)/test/ingopcs
### concatenate all srcs directories
C_SRC_DIRS=$(UASTACK_DIR) $(STUBCLIENT_DIR) $(STUBSERVER_DIR) $(TESTS_DIR)

## Stack
### includes stack
INCLUDES_UASTACK=$(shell find $(UASTACK_DIR) -not -path $(EXCLUDE_DIR) -type d)
## object files stack
UASTACK_SRC_FILES=$(shell find $(UASTACK_DIR) -not -path $(EXCLUDE_DIR) -type f -name "*.c" -exec basename "{}" \;)
UASTACK_OBJ_FILES=$(patsubst %.c,$(BUILD_DIR)/%.o,$(UASTACK_SRC_FILES))

## Tests object files
TESTS_SRC_FILES=$(shell find $(TESTS_DIR) -type f -name "*.c" -exec basename "{}" \;)
TESTS_OBJ_FILES=$(patsubst %.c,$(BUILD_DIR)/%.o,$(TESTS_SRC_FILES))

## All .c and .h files to compute dependencies
C_SRC_PATHS=$(shell find $(C_SRC_DIRS) -not -path $(EXCLUDE_DIR) -type f -name "*.c")
H_SRC_PATHS=$(shell find $(C_SRC_DIRS) -not -path $(EXCLUDE_DIR) -type f -name "*.h")

# MBEDTLS INPUTS
MBEDTLS_DIR=$(WORKSPACE_DIR)/lib/mbedtls-2.3.0
INCLUDES_MBEDTLS=-I$(MBEDTLS_DIR)/include
LIBS_MBEDTLS=-L$(MBEDTLS_DIR)/library -lmbedtls -lmbedx509 -lmbedcrypto

# C COMPILER CONFIG
CCFLAGS=-c -g -Wall -Wextra -O0 #-pedantic -std=c99
LFLAGS=-g
INCLUDES=$(INCLUDES_MBEDTLS) $(INCLUDES_SSL) $(addprefix -I, $(INCLUDES_UASTACK))
DEFS=-DOPCUA_USE_SYNCHRONISATION=0 -DOPCUA_MULTITHREADED=0 -DOPCUA_TRACE_ENABLE=1 #-DOPCUA_HAVE_SERVERAPI=1 -DOPCUA_HAVE_OPENSSL=1

# MAKEFILE CONTENT

.PHONY : all config mbedtls check clean clean_mbedtls cleanall
.DELETE_ON_ERROR : .depend

default: all

all: config $(EXEC_DIR)/stub_client $(EXEC_DIR)/stub_server $(EXEC_DIR)/check_stack

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include .depend
endif
endif

config: mbedtls
	@echo "Configuring build dirs..."
	@\mkdir -p $(BUILD_DIR) $(EXEC_DIR)
	@\mkdir -p $(EXEC_DIR)/revoked $(EXEC_DIR)/untrusted $(EXEC_DIR)/trusted \
	 $(EXEC_DIR)/client_private $(EXEC_DIR)/server_private \
	 $(EXEC_DIR)/client_public $(EXEC_DIR)/server_public
	@\cp $(CERT_DIR)/cacert.der $(EXEC_DIR)/trusted
	@\cp $(CERT_DIR)/client.key $(EXEC_DIR)/client_private
	@\cp $(CERT_DIR)/client.der $(EXEC_DIR)/client_public
	@\cp $(CERT_DIR)/server.key $(EXEC_DIR)/server_private
	@\cp $(CERT_DIR)/server.der $(EXEC_DIR)/server_public

$(BUILD_DIR)/%.o:
	@echo "  CC $@"
	@$(CC) $(CCFLAGS) $(INCLUDES) $< -o $@ $(DEFS)

.depend: $(C_SRC_PATHS) #$(H_SRC_PATHS)
	@echo "Building dependencies..."
	@$(CC) $(CCFLAGS) $(DEFS) $(INCLUDES) -MM $(C_SRC_PATHS) > .depend
	@sed 's/^\(.*\)\.o:/$(BUILD_DIR_SED)\/\1.o:/g' -i .depend

$(EXEC_DIR)/stub_server: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/stub_server.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS)
	@$(COPY_SSL)

$(EXEC_DIR)/stub_client: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/stub_client.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS)
	@$(COPY_SSL)

$(EXEC_DIR)/check_stack: $(UASTACK_OBJ_FILES) $(TESTS_OBJ_FILES) $(BUILD_DIR)/check_stack.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS) -lcheck -lm

mbedtls:
	@echo "Building mbedtls..."
	@$(MAKE) -C $(MBEDTLS_DIR)

check: config $(EXEC_DIR)/check_stack
	@echo "Executing tests..."
	@\cd $(EXEC_DIR) && ./check_stack

clean_mbedtls:
	@echo "Cleaning mbedtls"
	@$(MAKE) -C $(MBEDTLS_DIR) clean

clean:
	@echo "Cleaning..."
	@\rm -rf $(BUILD_DIR) $(EXEC_DIR)
	@\rm -f .depend

cleanall: clean clean_mbedtls
