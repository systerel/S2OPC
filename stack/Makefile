################################################################################
#     INGOPCS Stack Makefile
#
#     Specific variables that could be used (optional): "make <VAR>=<value> ..."
#     - CHECK_PREFIX=<path-to-checklib-install> (containing include/ and lib/) 
#        => no need if installed in system standard path
#     - INSTALL_PREFIX=<path-install> (that will contain include/ and lib/):
#       * make library                               => library generation
#       * (sudo) make INSTALL_PREFIX=<path> install  => library installation
#     - STACK_1_01=<any-value> or STACK_1_02=<any-value>
#       or STACK_1_03=<any-value> (default): set API wrapper version to use
#     - CROSS_COMPILE_WIN=<any-value>: compilation with i686-w64-mingw32 compiler
#
#     Library generation and installation (linux only):
#     - make library
#     - sudo make
################################################################################

# OS DEPENDENCIES

OSTYPE=$(shell echo $$OSTYPE)
ifdef CROSS_COMPILE_WIN
     OSTYPE=cc_win
     export CROSS_COMPILE_WIN
endif

ifdef CHECK_PREFIX
    # CHECK_PREFIX must be defined for windows: path to libcheck installation
    LIB_CHECK=-L$(CHECK_PREFIX)/lib
    INCLUDE_CHECK=$(CHECK_PREFIX)/include
    INCLUDES_OTHERS=-I$(INCLUDE_CHECK)
else
    LIB_CHECK=
    INCLUDE_CHECK=
    INCLUDES_OTHERS=
endif

ifeq ($(OSTYPE),$(filter linux% darwin%,$(OSTYPE)))
     CC=gcc
     AR=ar
     STATIC_LIB_EXT=".a"
     DYNAMIC_LIB_EXT=".so"
     EXCLUDE_DIR="*win*"
     PLATFORM_DIR="*linux*"
     PFLAGS=-std=c99 -pedantic -D_XOPEN_SOURCE=600
     LIBS=$(LIBS_MBEDTLS) -lpthread -lrt $(LIB_CHECK)
else
    STATIC_LIB_EXT=".a"
    DYNAMIC_LIB_EXT=".so"
    EXCLUDE_DIR="*linux*"
    PLATFORM_DIR="*win*"
    PFLAGS=-std=c99 -pedantic
    LIBS=$(LIBS_MBEDTLS) -lrpcrt4 -lws2_32 $(LIB_CHECK)
    DEF_WINDOWS=-D_WIN32_WINNT=0x0600 # minimum Vista for IPV6 support
    WINDOWS=1
    export WINDOWS
ifdef CROSS_COMPILE_WIN
    CC=i686-w64-mingw32-gcc
    AR=i686-w64-mingw32-ar
else
    CC=gcc
    AR=ar
endif
endif

export CC
export AR

ifdef STACK_1_01
    DEF_STACK=-DSTACK_1_01 
else
    ifdef STACK_1_02
       DEF_STACK=-DSTACK_1_02 
    else
       DEF_STACK=-DSTACK_1_03 
    endif
endif

ifdef SHARED
    SHARED_FLAG=-fPIC
    MBED_SHARED="SHARED=yes"
endif

# OUTPUTS
WORKSPACE_DIR=.
ifndef EXEC_DIR
    EXEC_DIR=$(WORKSPACE_DIR)/out
endif
CERT_DIR=$(WORKSPACE_DIR)/cert
BUILD_DIR=$(WORKSPACE_DIR)/build
BUILD_DIR_SED=$(subst /,\/,$(BUILD_DIR))
ifndef LOCAL_INSTALL_DIR
    LOCAL_INSTALL_DIR=$(WORKSPACE_DIR)/install
endif

# INPUTS

## Directories
### srcs directories definition
UASTACK_DIR=$(WORKSPACE_DIR)/src
STUBCLIENT_DIR=$(WORKSPACE_DIR)/stub_client
STUBSERVER_DIR=$(WORKSPACE_DIR)/stub_server
INTERNAL_TESTS_DIR=$(WORKSPACE_DIR)/test/ingopcs
### concatenate all srcs directories
C_SRC_DIRS=$(UASTACK_DIR) $(INTERNAL_TESTS_DIR) # source files with access to any headers
C_STUBS_DIRS=$(STUBCLIENT_DIR) $(STUBSERVER_DIR) # source files with access to public headers only

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

BUILD_STUBS_DIR=$(BUILD_DIR)_stubs
BUILD_STUBS_DIR_SED=$(subst /,\/,$(BUILD_STUBS_DIR))

# add platform objs prior to stack objs
UASTACK_OBJ_FILES=$(PLATFORM_OBJ_FILES) $(STACK_OBJ_FILES)

## Tests object files
INTERNAL_TESTS_SRC_FILES=$(shell find $(INTERNAL_TESTS_DIR) -type f -name "*.c" -exec basename "{}" \;)
INTERNAL_TESTS_OBJ_FILES=$(patsubst %.c,$(BUILD_DIR)/%.o,$(INTERNAL_TESTS_SRC_FILES))

## All .c and .h files to compute dependencies
C_SRC_PATHS=$(shell find $(C_SRC_DIRS) -not -path $(EXCLUDE_DIR) -type f -name "*.c")
C_STUBS_SRC_PATHS=$(shell find $(C_STUBS_DIRS) -type f -name "*.c")
H_SRC_PATHS=$(shell find $(C_SRC_DIRS) -not -path $(EXCLUDE_DIR) -type f -name "*.h")
H_INCLUDE_PATHS=$(H_SRC_PATHS)

# MBEDTLS INPUTS
MBEDTLS_DIR=$(WORKSPACE_DIR)/../libraries/mbedtls-2.3.0
INCLUDES_MBEDTLS=-I$(MBEDTLS_DIR)/include
LIBS_MBEDTLS=-L$(MBEDTLS_DIR)/library -lmbedtls -lmbedx509 -lmbedcrypto

# C COMPILER CONFIG
CFLAGS=-c -g -Wall -Wextra -O0 $(SHARED_FLAG)
C99FLAGS=-std=c99 -pedantic
LFLAGS=-g
INCLUDES=$(INCLUDES_MBEDTLS) $(addprefix -I, $(INCLUDES_UASTACK)) $(INCLUDES_OTHERS)
DEFS=$(DEF_STACK) $(DEF_THREAD) $(DEF_WINDOWS)

# MAKEFILE CONTENT

.PHONY : all doc config config_stubs mbedtls library install check clean clean_doc clean_mbedtls cleanall
.DELETE_ON_ERROR : .depend.tmp .depend .pdepend

default: all

all: config library $(EXEC_DIR)/stub_client_ingopcs $(EXEC_DIR)/stub_server_ingopcs $(EXEC_DIR)/check_stack

ifndef MAKECMDGOALS
    MAKECMDGOALS=default
endif
ifneq ($(MAKECMDGOALS),$(filter $(MAKECMDGOALS), clean clean_doc clean_mbedtls cleanall config config_stubs doc install))
-include .depend
-include .pdepend
-include .tdepend
endif

doc:
	@echo "Generating documentation in apidoc/ with doxygen"
	@doxygen doxygen/ingopcs-stack.doxyfile -DOPCUA_HAVE_CLIENTAPI=1 -DOPCUA_HAVE_SERVERAPI=1

config: mbedtls
	@echo "Configuring build and output directory..."
	@"mkdir" -p $(BUILD_DIR) $(BUILD_STUBS_DIR) $(PLATFORM_BUILD_DIR) $(EXEC_DIR)

config_stubs: config
	@echo "Configuring stubs tests data..."
	@"mkdir" -p $(EXEC_DIR)/revoked $(EXEC_DIR)/untrusted $(EXEC_DIR)/trusted \
	 $(EXEC_DIR)/client_private $(EXEC_DIR)/server_private \
	 $(EXEC_DIR)/client_public $(EXEC_DIR)/server_public
	@"cp" $(CERT_DIR)/cacert.der $(EXEC_DIR)/trusted
	@"cp" $(CERT_DIR)/client.key $(EXEC_DIR)/client_private
	@"cp" $(CERT_DIR)/client.key $(EXEC_DIR)/client_private/client.pem
	@"cp" $(CERT_DIR)/client.der $(EXEC_DIR)/client_public
	@"cp" $(CERT_DIR)/server.key $(EXEC_DIR)/server_private
	@"cp" $(CERT_DIR)/server.der $(EXEC_DIR)/server_public


$(BUILD_DIR)/%.o:
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(C99FLAGS) $(INCLUDES) $< -o $@ $(DEFS)

$(BUILD_STUBS_DIR)/%.o:
	@echo "  CC $@"
	@$(CC) $(CFLAGS) $(C99FLAGS) -I$(LOCAL_INSTALL_DIR)/include $< -o $@ $(DEFS)

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

.tdepend: $(C_STUBS_SRC_PATHS)
	@echo "Building stub tests dependencies..."
	@$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -MM $(C_STUBS_SRC_PATHS) > $@.tmp
	@sed 's/^\(.*\)\.o:/$(BUILD_STUBS_DIR_SED)\/\1.o:/g' $@.tmp > $@

$(EXEC_DIR)/stub_client_ingopcs: config_stubs library $(BUILD_STUBS_DIR)/stub_client_ingopcs.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) -I $(LOCAL_INSTALL_DIR)/include $(BUILD_STUBS_DIR)/stub_client_ingopcs.o -o $@ -L$(LOCAL_INSTALL_DIR)/lib -lingopcs $(LIBS)

$(EXEC_DIR)/stub_server_ingopcs: config_stubs library $(BUILD_STUBS_DIR)/stub_server_ingopcs.o
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) -I $(LOCAL_INSTALL_DIR)/include $(BUILD_STUBS_DIR)/stub_server_ingopcs.o -o $@ -L$(LOCAL_INSTALL_DIR)/lib -lingopcs $(LIBS)

$(EXEC_DIR)/check_stack: config $(INTERNAL_TESTS_OBJ_FILES)
	@echo "Linking $@..."
	@$(CC) $(LFLAGS) $(INCLUDES) $(UASTACK_OBJ_FILES) $(INTERNAL_TESTS_OBJ_FILES) -o $@ $(LIBS) -lcheck

library: config $(UASTACK_OBJ_FILES)
	@echo "Generating static library in $(LOCAL_INSTALL_DIR)/lib"
	@"mkdir" -p $(LOCAL_INSTALL_DIR)/lib
	@$(AR) -rc $(LOCAL_INSTALL_DIR)/lib/libingopcs$(STATIC_LIB_EXT) $(UASTACK_OBJ_FILES)
	@$(AR) -s $(LOCAL_INSTALL_DIR)/lib/libingopcs$(STATIC_LIB_EXT)
ifdef SHARED
	@$(CC) -shared -o $(LOCAL_INSTALL_DIR)/lib/libingopcs$(DYNAMIC_LIB_EXT) -Wl,--whole-archive $(LOCAL_INSTALL_DIR)/lib/libingopcs$(STATIC_LIB_EXT) -Wl,--no-whole-archiv
endif
	@echo "Copying headers to includes in $(LOCAL_INSTALL_DIR)/include"
	@"mkdir" -p $(LOCAL_INSTALL_DIR)/include
	@"cp" $(H_INCLUDE_PATHS) $(LOCAL_INSTALL_DIR)/include

install:
	@echo "Install INGOPCS library files into $(INSTALL_PREFIX)"
	@"cp" -r $(LOCAL_INSTALL_DIR)/* $(INSTALL_PREFIX)/

client_server_test: $(EXEC_DIR)/stub_client_ingopcs $(EXEC_DIR)/stub_server_ingopcs
	./run_client_server_test.sh $(EXEC_DIR)

mbedtls:
	@echo "Building mbedtls..."
	@$(MAKE) $(MBED_SHARED) -C $(MBEDTLS_DIR) lib

check: config library $(EXEC_DIR)/check_stack
	@echo "Executing tests..."
	@\cd $(EXEC_DIR) && ./check_stack

clean_mbedtls:
	@echo "Cleaning mbedtls"
	@$(MAKE) -C $(MBEDTLS_DIR) clean

clean_doc:
	@echo "Cleaning documentation..."
	@"rm" -rf apidoc

clean:
	@echo "Cleaning..."
	@"rm" -rf "./$(BUILD_DIR)" "./$(BUILD_STUBS_DIR)" "./$(PLATFORM_BUILD_DIR)" "./$(EXEC_DIR)"
	@"rm" -rf "./$(LOCAL_INSTALL_DIR)"
	@"rm" -f .depend.tmp .depend .pdepend .tdepend.tmp .tdepend

cleanall: clean clean_doc clean_mbedtls
