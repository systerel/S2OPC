# OS DEPENDENCIES
OSTYPE=$(shell echo $$OSTYPE)

ifeq ($(OSTYPE),$(filter linux% darwin%,$(OSTYPE)))
     CC=gcc
     EXCLUDE_DIR="*win*"
     PLATFORM_DIR="*linux*"
     PFLAGS=-std=c99 -pedantic -D_XOPEN_SOURCE=600
     LIBS=$(LIBS_MBEDTLS) -lpthread
else
    CC=i686-w64-mingw32-gcc #i686-pc-mingw32-gcc
    EXCLUDE_DIR="*linux*"
    PLATFORM_DIR="*win*"
    PFLAGS=""
    LIBS=$(LIBS_MBEDTLS) -lrpcrt4 -lws2_32
endif

ifdef STACK_1_01
    DEF_STACK=-DSTACK_1_01 
else
    ifdef STACK_1_02
       DEF_STACK=-DSTACK_1_02 
    else
       DEF_STACK=-DSTACK_1_03 
    endif
endif

ifdef WRAPPER_RECEPTION_THREAD
    DEF_THREAD=-DWRAPPER_RECEPTION_THREAD
endif

ifdef SHARED
    SHARED_FLAG=-fPIC
    MBED_SHARED="SHARED=yes"
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
STUBCLIENT_DIR=$(WORKSPACE_DIR)/stub_client
STUBSERVER_DIR=$(WORKSPACE_DIR)/stub_server
TESTS_DIR=$(WORKSPACE_DIR)/test/ingopcs
### concatenate all srcs directories
C_SRC_DIRS=$(UASTACK_DIR) $(STUBCLIENT_DIR) $(STUBSERVER_DIR) $(TESTS_DIR)

## Stack
### includes stack
INCLUDES_UASTACK=$(shell find $(UASTACK_DIR) -not -path $(EXCLUDE_DIR) -type d)
## object files stack
STACK_SRC_FILES=$(shell find $(UASTACK_DIR) -not -path $(EXCLUDE_DIR) -not -path $(PLATFORM_DIR) -type f -name "*.c" -exec basename "{}" \;)
STACK_OBJ_FILES=$(patsubst %.c,$(BUILD_DIR)/%.o,$(STACK_SRC_FILES))

PLATFORM_BUILD_DIR=$(BUILD_DIR)_platform
PLATFORM_BUILD_DIR_SED=$(subst /,\/,$(PLATFORM_BUILD_DIR))
PLATFORM_SRC_FILES=$(shell find $(UASTACK_DIR) -path $(PLATFORM_DIR) -type f -name "*.c" -exec basename "{}" \;)
PLATFORM_OBJ_FILES=$(patsubst %.c,$(PLATFORM_BUILD_DIR)/%.o,$(PLATFORM_SRC_FILES))

# add platform objs prior to stack objs
UASTACK_OBJ_FILES=$(PLATFORM_OBJ_FILES) $(STACK_OBJ_FILES)

## Tests object files
TESTS_SRC_FILES=$(shell find $(TESTS_DIR) -type f -name "*.c" -exec basename "{}" \;)
TESTS_OBJ_FILES=$(patsubst %.c,$(BUILD_DIR)/%.o,$(TESTS_SRC_FILES))

## All .c and .h files to compute dependencies
C_SRC_PATHS=$(shell find $(C_SRC_DIRS) -not -path $(EXCLUDE_DIR) -type f -name "*.c")
H_SRC_PATHS=$(shell find $(C_SRC_DIRS) -not -path $(EXCLUDE_DIR) -type f -name "*.h")
H_INCLUDE_PATHS=$(shell find $(UASTACK_DIR)/API $(UASTACK_DIR)/APIwrappers $(UASTACK_DIR)/core_types -type f -name "*.h")

# MBEDTLS INPUTS
MBEDTLS_DIR=$(WORKSPACE_DIR)/lib/mbedtls-2.3.0
INCLUDES_MBEDTLS=-I$(MBEDTLS_DIR)/include
LIBS_MBEDTLS=-L$(MBEDTLS_DIR)/library -lmbedtls -lmbedx509 -lmbedcrypto

# C COMPILER CONFIG
CFLAGS=-c -g -Wall -Wextra -O0 $(SHARED_FLAG)
C99FLAGS=-std=c99 -pedantic
LFLAGS=-g
INCLUDES=$(INCLUDES_MBEDTLS) $(addprefix -I, $(INCLUDES_UASTACK))
DEFS=$(DEF_STACK) $(DEF_THREAD)

# MAKEFILE CONTENT

.PHONY : all doc config mbedtls check clean clean_mbedtls cleanall
.DELETE_ON_ERROR : .depend.tmp .depend .pdepend

default: all

all: config lib/libingopcs.a $(EXEC_DIR)/stub_client_ingopcs $(EXEC_DIR)/stub_server_ingopcs $(EXEC_DIR)/check_stack

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
ifneq ($(MAKECMDGOALS),doc)
-include .depend
-include .pdepend
endif
endif
endif

doc:
	@echo "Generating documentation in apidoc/ with doxygen"
	@doxygen doxygen/ingopcs-stack.doxyfile -DOPCUA_HAVE_CLIENTAPI=1

config: mbedtls
	@echo "Configuring build dirs..."
	@\mkdir -p $(BUILD_DIR) $(PLATFORM_BUILD_DIR) $(EXEC_DIR)
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
	@$(CC) $(CFLAGS) $(C99FLAGS) $(INCLUDES) $< -o $@ $(DEFS)

$(PLATFORM_BUILD_DIR)/%.o:
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(PFLAGS) $(INCLUDES) $< -o $@ $(DEFS)

.depend: $(C_SRC_PATHS) #$(H_SRC_PATHS)
	@echo "Building dependencies..."
	@$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -MM $(C_SRC_PATHS) > $@.tmp
	@sed 's/^\(.*\)\.o:/$(BUILD_DIR_SED)\/\1.o:/g' $@.tmp > $@

.pdepend: .depend
	@echo "Building platform dependencies..."
	@sed 's/^\(.*\)\.o:/$(PLATFORM_BUILD_DIR_SED)\/\1.o:/g' $^.tmp > $@

$(EXEC_DIR)/stub_client_ingopcs: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/stub_client_ingopcs.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS)

$(EXEC_DIR)/stub_server_ingopcs: $(UASTACK_OBJ_FILES) $(BUILD_DIR)/stub_server_ingopcs.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS)

$(EXEC_DIR)/check_stack: $(UASTACK_OBJ_FILES) $(TESTS_OBJ_FILES) $(BUILD_DIR)/check_stack.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $^ -o $@ $(LIBS_DIR) $(LIBS) -lcheck -lm

lib/libingopcs.a: $(UASTACK_OBJ_FILES)
	@echo "Generating static library"
	@ar -rc $@ $^
	@ar -s $@
	@echo "Copying headers to includes in include"
	@"mkdir" -p include
	@"cp" $(H_INCLUDE_PATHS) include/

client_server_test: $(EXEC_DIR)/stub_client_ingopcs $(EXEC_DIR)/stub_server_ingopcs
	./run_client_server_test.sh

mbedtls:
	@echo "Building mbedtls..."
	@$(MAKE) $(MBED_SHARED) -C $(MBEDTLS_DIR) programs tests

check: $(EXEC_DIR)/check_stack
	@echo "Executing tests..."
	@\cd $(EXEC_DIR) && ./check_stack

clean_mbedtls:
	@echo "Cleaning mbedtls"
	@$(MAKE) -C $(MBEDTLS_DIR) clean

clean:
	@echo "Cleaning..."
	@"rm" -rf $(BUILD_DIR) $(PLATFORM_BUILD_DIR) $(EXEC_DIR) apidoc
	@"rm" -rf include lib/libingopcs.a
	@"rm" -f .depend.tmp .depend .pdepend

cleanall: clean clean_mbedtls
