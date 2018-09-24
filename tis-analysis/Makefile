##########################################################################
#                                                                        #
#  This file is part of deliverable T3.3 of project INGOPCS              #
#                                                                        #
#    Copyright (C) 2017 TrustInSoft                                      #
#                                                                        #
#  All rights reserved.                                                  #
#                                                                        #
##########################################################################

ifeq ($(shell which tis-analyzer),)
  $(error You need a TrustInSoft box to run the analysis)
endif

TOOLS_DIR=$(dir $(shell which tis-analyzer))../kit-tools
include $(TOOLS_DIR)/targets.mk

#===============================================================================
# Files and Directories
#-------------------------------------------------------------------------------

# TIS_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
TIS_DIR := .

# sources is a link to .. because .. cannot be used here
# otherwise, we got absolute paths in the oracles, which is not good.
SRC_DIR:=$(TIS_DIR)/sources

FRAMAC_SHARE:=$(shell tis-kernel-config -print-share-path)
FRAMAC_LIBC:=$(FRAMAC_SHARE)/libc

# LOCAL_LIBC: place to add missing headers.
LOCAL_LIBC:=$(TIS_DIR)/external_headers

# TIS_SRC_DIR: place to add custom source files such as main, etc
TIS_SRC_DIR:=$(TIS_DIR)/tis_src

# how to move to the root of the repo
TOP_DIR:=$(TIS_DIR)/..

# the place to execute the tests
BIN_DIR:=$(TOP_DIR)/bin

# RES: standard place to put the results AND the oracles.
RES:=$(TIS_DIR)/results

# the path from BIN_DIR to TIS_DIR
BIN_TO_TIS_DIR:=../tis-analysis

#-------------------------------------------------------------------------------

CHECK_DIR?=$(TIS_DIR)/../../check_lib
INCLUDES_CHECK:=$(CHECK_DIR)/include

#--------------

MBEDTLS_DIR?=$(TIS_DIR)/../../mbedtls
MBEDTLS_SRC_FILES=$(shell find $(MBEDTLS_DIR)/library -type f -name "*.c" )
INCLUDES_MBEDTLS=$(MBEDTLS_DIR)/include

#--------------

STACK_SRC_FILES := \
  $(shell find $(SRC_DIR)/csrc -type f -name "*.c" | grep -v windows )

INCLUDES_STACK := \
  $(shell find $(SRC_DIR)/csrc -type d | grep -v windows )
INCLUDES_STACK += $(SRC_DIR)/address_space_generation/include
INCLUDES_STACK += $(SRC_DIR)/install_linux/include

#--------------

SRC_INCLUDES=$(INCLUDES_MBEDTLS) $(INCLUDES_STACK) $(INCLUDES_CHECK)
ALL_SRC_FILES:= $(STACK_SRC_FILES) # $(MBEDTLS_SRC_FILES)

#===============================================================================
# Compilation Options
#-------------------------------------------------------------------------------

DEFINES:=\
  __TRUSTINSOFT_BUGFIX__ \
  __TRUSTINSOFT_HELPER__ \
  __TRUSTINSOFT_LABEL__ \
  __TRUSTINSOFT_TMPBUG__ \


# for tis-interpreter
TI_CC_ARGS:=\
  -isystem $(LOCAL_LIBC) \
  -isystem $(FRAMAC_SHARE)/.. \
  -isystem $(FRAMAC_SHARE)/mt \
  $(addprefix -I, $(SRC_INCLUDES)) \
  $(addprefix -D, $(DEFINES)) -D__TRUSTINSOFT_INTERPRETER__ \
  -DUSE_MBEDTLS_SRCS \
  -Dvolatile= \

SHARED_OPTIONS := \
  -from-verbose 0 \
  -no-val-warn-harmless-function-pointers \
  -acsl-import $(TIS_SRC_DIR)/empty.acsl

TI_ARGS := \
  --cc \"$(TI_CC_ARGS)\" \
  $(SHARED_OPTIONS) \

TI_ARGS_WITH_RESULTS = $(TI_ARGS) -server -val -no-server

TIS_ARGS:=\
  -64 \
  -isystem $(LOCAL_LIBC) \
  $(SHARED_OPTIONS) \
  -isystem $(FRAMAC_SHARE)/mt \
  $(addprefix -I , $(SRC_INCLUDES)) \
  $(addprefix -D , $(DEFINES))

# TIS_ARGS_WITH_MBEDTLS := \
#   -acsl-import $(TIS_SRC_DIR)/mbedtls_properties.acsl \
#   -D WITH_MBEDTLS_SRCS $(TIS_SRC_DIR)/mbedtls_stubs.c \

TIS_ARGS_WITHOUT_MBEDTLS := \
  -acsl-import $(TIS_SRC_DIR)/mbedtls_spec.acsl \
  -D __TRUSTINSOFT_NO_MBEDTLS__ \
  -D WITHOUT_MBEDTLS_SRCS $(TIS_SRC_DIR)/mbedtls_stubs.c \

SLEVEL ?= 1000
VAL_OPTIONS = \
  -val -slevel $(SLEVEL) \
  -val-stop-at-nth-garbled 1 \
  -val-split-return full \
  -val-slevel-merge-after-loop=-@all \
  -no-val-warn-pointer-arithmetic-out-of-bounds \
  -val-builtin sscanf:tis_sscanf \
  -val-malloc-functions calloc \
  -no-val-print

ifneq ($(STOP_NTH),)
  VAL_OPTIONS += -val-stop-at-nth-alarm $(STOP_NTH)
endif

#===============================================================================
# Targets
#-------------------------------------------------------------------------------

.PHONY: default
default:
	@echo "== Which target?"
	@echo
	@echo "# Tests Analysis:"
	@echo "- make ti_tests: tis-interpreter on test/ingopcs;"
	@echo "- make tests: tis-analyzer on test/ingopcs;"
	@echo "- make tests-list: show the tests list;"
	@echo
	@echo "# Client Analysis:"
# 	@echo "- make ti_client: tis-interpreter on stub_client;"
# 	@echo "- make ti_client_state: tis-interpreter on stub_client\
# 	  with results (state can be opened in GUI);"
# 	@echo "- make client: tis-analyzer on stub_client;"
	@echo "- make tk_client: tis-analyzer on toolkit_client;"
	@echo "- make sch_client: tis-analyzer on secure_channels_client;"
	@echo
	@echo "# Others"
	@echo "- make all: main checks"
	@echo "- make wp: WP proofs"
	@echo "- make cov: overall coverage from all tests"
	@echo "- make html_cov: overall coverage from all tests in HTML"
	@echo "- make modifications: check the modifications with git"
	@echo "- make modif: check modifications inside TRUSTINSOFT macros"
	@echo "- make clean"
	@echo "- make compile: build INGOPCS binaries"

all: tests cov wp modifications

#-------------------------------------------------------------------------------
# ACLS files dependencies
#-------------------------------------------------------------------------------

$(TIS_SRC_DIR)/mbedtls_properties.acsl: $(TIS_SRC_DIR)/mbedtls_spec.acsl
	touch $@

#-------------------------------------------------------------------------------
# Compilation:
#-------------------------------------------------------------------------------

.PHONY: compile_mbedtls
compile_mbedtls: $(MBEDTLS_DIR)/library/libmbedtls.a

$(MBEDTLS_DIR)/library/libmbedtls.a:
	$(ECHO_CIBLE)
	cd $(MBEDTLS_DIR) && make

GENERATED += $(MBEDTLS_DIR)/library/libmbedtls.a

.PHONY: compile_check_lib
compile_check_lib: $(CHECK_DIR)

$(CHECK_DIR):
	$(ECHO_CIBLE)
	$(TIS_DIR)/../../build_check_lib.sh

GENERATED += $(CHECK_DIR)

BINARIES := $(BIN_DIR)/toolkit_test_client
BINARIES += $(BIN_DIR)/test_secure_channels_client
BINARIES += $(BIN_DIR)/check_helpers

$(BINARIES): $(MBEDTLS_DIR)/library/libmbedtls.a $(CHECK_DIR) $(STACK_SRC_FILES)
	$(ECHO_CIBLE)
	cd $(TOP_DIR) ; rm -rf build ; ./build.sh

GENERATED += $(BINARIES)

.PHONY: compile_ingopcs
compile_ingopcs: $(BINARIES)

#-------------------------------------------------------------------------------
# Analyses:
#-------------------------------------------------------------------------------

TESTS_DIR=$(SRC_DIR)/tests
TEST_NAMES := addspace test_read test_write helpers tk_client sch_client \
            sockets
TI_TESTS := ti_addspace ti_test_read ti_test_write \
          # ti_helpers ti_sockets

TESTS := $(TEST_NAMES) $(TI_TESTS)

.PHONY: tests ti_tests $(TESTS)
tests: $(TESTS)
ti_tests: $(TI_TESTS)

tests-list:
	@echo "tests: $(TEST_NAMES)"
	@echo "ti_tests: $(TI_TESTS)"

$(TEST_NAMES) $(TI_TESTS): %: $(RES)/%_check.ok $(RES)/%.meta
$(MTHREAD_TESTS): %: $(RES)/%.state $(RES)/%.meta

BASES+=$(TESTS:%=$(RES)/%)

$(BASES:%=%.state): $(CHECK_DIR)

$(TEST_NAMES:%=$(RES)/%.state): $(RES)/%.state: $(STACK_SRC_FILES) \
                                                # $(TIS_DIR)/Makefile
	$(ECHO_CIBLE)
	source $(TOOLS_DIR)/run.sh; \
	  TOOLS_DIR=$(TOOLS_DIR) run_analyzer \
	  \
	  $(TIS_ARGS) \
	  \
	  $(VAL_OPTIONS) \
	  \
	  $(TEST_ARGS) \
	  \
	  $(TEST_SRCS) \
	  \
	  -then -save $(RES)/$*.state > $(RES)/$*.log

# tis-interpreter
$(TI_TESTS:%=$(RES)/%.state): $(RES)/%.state: $(STACK_SRC_FILES) \
                                              # $(TIS_DIR)/Makefile
	$(ECHO_CIBLE)
	source $(TOOLS_DIR)/run.sh; \
	  TOOL=tis-interpreter \
	  TOOLS_DIR=$(TOOLS_DIR) run_analyzer \
	  $(TI_ARGS_WITH_RESULTS) $(TEST_TI_ARGS) \
	  \
	  $(TEST_SRCS) \
	  \
	  -then -save $(RES)/$*.state > $(RES)/$*.log

.PHONY: acg
acg:
	@echo "To build a callgraph, start with command below and add source files and other -acg options:"
	@echo
	@echo "tis-analyzer $(TIS_ARGS) $(TEST_ARGS) -add-path ~/Perso/GitTom/fc-anne/afp-callgraph/ -acg"
	@echo
	@echo "to include all files :"
	@echo "(find $(SRC_DIR)/csrc \
          -path  $(SRC_DIR)/csrc/helpers_platform_dep/windows -prune -o \
	  -type f -name '*.c')"

#-------------------------------------------------------------------------------

TEST_HELPERS_SRCS := \
  $(RES)/helpers_inputs.c \
  $(TIS_SRC_DIR)/stubs.c \
  $(shell find $(TESTS_DIR)/helpers -type f -name "*.c") \
  $(TIS_SRC_DIR)/tis_check_lib.c \
  $(shell find $(SRC_DIR)/csrc/helpers -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/crypto -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/opcua_types -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/helpers_platform_dep/linux -type f -name "*.c") \
  $(SRC_DIR)/csrc/helpers_platform_dep/sopc_time.c \
  $(TIS_SRC_DIR)/tis_time.c \

$(RES)/check_helpers.log: $(BIN_DIR)/check_helpers
	$(ECHO_CIBLE)
	cd $(BIN_DIR) ; ./check_helpers > $(BIN_TO_TIS_DIR)/$@

$(RES)/helpers_inputs.c: $(TIS_DIR)/scripts/tis-inputs.awk
$(RES)/helpers_inputs.c: $(RES)/check_helpers.log
	$(ECHO_CIBLE)
	gawk -f $(TIS_DIR)/scripts/tis-inputs.awk $< > $@

$(RES)/helpers.state : $(TEST_HELPERS_SRCS) \
                       $(TIS_SRC_DIR)/mbedtls_stubs.c \
                       $(TIS_SRC_DIR)/mbedtls_spec.acsl \


$(RES)/helpers.state : TEST_SRCS = $(TEST_HELPERS_SRCS)

$(RES)/helpers.state: TEST_ARGS += \
  $(TIS_ARGS_WITHOUT_MBEDTLS) \
  -D __TRUSTINSOFT_SKIP_CRYPTO_TESTS__ \
  -D __TRUSTINSOFT_DEBUG__ -D SOPC_DEBUG_PRINTING=true \
  \
  -D SOPC_MAX_DIAG_INFO_NESTED_LEVEL=3 -val-clone-on-recursive-calls \
  -val-malloc-returns-null \
  -val-malloc-plevel 15 \
  -val-stop-at-nth-alarm 1 \
  -slevel-function tis_memcpy_bounded:2000 \
  -slevel-function unhexlify:2000 \
  -slevel-function hexlify:2000 \
  # \
  -memexec-all \
  The slevel below makes the analysis much longter (does it terminate?) \
  but the 'escaping' problem doesn't show up ! \
  -slevel-function CryptoProvider_AsymEncrypt_RSA_OAEP:5000 \
  -slevel-function CryptoProvider_AsymDecrypt_RSA_OAEP:5000 \

#-------------------------------------------------------------------------------

ADDR_SPACE_SRC_FILES := \
  $(shell find $(SRC_DIR)/address_space_generation -type f -name "*.c" ) \

ADDR_SPACE_LIB_FILES := \
  $(filter-out \
    $(SRC_DIR)/address_space_generation/genc/check_ingopcs_addspace.c, \
    $(ADDR_SPACE_SRC_FILES))

$(RES)/addspace.state $(RES)/ti_addspace.state: $(ADDR_SPACE_SRC_FILES)
$(RES)/addspace.state $(RES)/ti_addspace.state: \
  TEST_SRCS += $(ADDR_SPACE_SRC_FILES)


#-------------------------------------------------------------------------------

TEST_READ_SRC:= \
  $(ADDR_SPACE_LIB_FILES) \
  $(SRC_DIR)/tests/services/toolkit_test_read.c \
  $(SRC_DIR)/tests/services/testlib_read_response.c \
  $(SRC_DIR)/tests/services/wrap_read.c \
  $(shell find $(SRC_DIR)/csrc/configuration -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/services -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/opcua_types -type f -name "*.c") \

$(RES)/test_read.state $(RES)/ti_test_read.state: \
  $(TEST_READ_SRC) $(TIS_DIR)/Makefile
$(RES)/test_read.state $(RES)/ti_test_read.state: TEST_SRCS += $(TEST_READ_SRC)
$(RES)/test_read.state: TEST_ARGS += \
  -no-val-malloc-returns-null \
  # \
  # Cannot use -val-malloc-returns-null because leads to malloc in a loop
  # in service_read__fill_read_response and the widdening mix up everything...
  # See also address_space_bs__read_AddressSpace_Attribute_value
  # that calls util_variant__new_Variant_from_XXX functions
  # which do the dynamic allocations.
  # \
  -val-malloc-returns-null \
  -slevel-function test_read_request_response:10000 \
  -slevel 5000 \
  -val-stop-at-nth-alarm 1 \
  -slevel-function service_read__fill_read_response:10000 \

#-------------------------------------------------------------------------------
TEST_WRITE_SRC:= \
  $(ADDR_SPACE_LIB_FILES) \
  $(SRC_DIR)/tests/services/toolkit_test_write.c \
  $(SRC_DIR)/tests/services/testlib_write.c \
  $(shell find $(SRC_DIR)/csrc/configuration -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/services -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/opcua_types -type f -name "*.c") \
  $(SRC_DIR)/csrc/helpers/sopc_logger.c \
  $(TIS_SRC_DIR)/stubs.c \

$(RES)/test_write.state $(RES)/ti_test_write.state: $(TEST_WRITE_SRC)
$(RES)/test_write.state $(RES)/ti_test_write.state: TEST_SRCS += \
  $(TEST_WRITE_SRC)
$(RES)/test_write.state $(RES)/ti_test_write.state: TEST_ARGS += \
  -val-malloc-returns-null \
  -val-stop-at-nth-alarm 1 \
  # \

#-------------------------------------------------------------------------------
TEST_TK_CLIENT_SRC:= \
  $(SRC_DIR)/tests/services/toolkit_test_client.c \
  $(SRC_DIR)/tests/services/testlib_write.c \
  $(SRC_DIR)/tests/services/test_results.c \
  $(SRC_DIR)/address_space_generation/genc/ingopcs_addspace.c \
  $(RES)/tk_client_inputs.c \
  $(TIS_SRC_DIR)/stubs.c \
  $(TIS_SRC_DIR)/tis_time.c \
  $(STACK_SRC_FILES)

$(RES)/%_client.log: $(BIN_DIR)/%_client
	$(ECHO_CIBLE)
	cd $(BIN_DIR) ; \
	  ( ./$*_server > $(BIN_TO_TIS_DIR)/$(RES)/$*_server.log & ) ; \
	  ( ./$*_client > $(BIN_TO_TIS_DIR)/$(RES)/$*_client.log )

$(RES)/tk_client_inputs.c: $(TIS_DIR)/scripts/tis-inputs.awk
$(RES)/tk_client_inputs.c: $(RES)/toolkit_test_client.log
	$(ECHO_CIBLE)
	gawk -f $(TIS_DIR)/scripts/tis-inputs.awk $< > $@

$(RES)/tk_client.state: $(TEST_TK_CLIENT_SRC) \
                       $(TIS_SRC_DIR)/mbedtls_stubs.c \
                       $(TIS_SRC_DIR)/mbedtls_spec.acsl \

$(RES)/tk_client.state: TEST_SRCS += $(TEST_TK_CLIENT_SRC)

$(RES)/tk_client.state: TEST_ARGS += \
  $(TIS_ARGS_WITHOUT_MBEDTLS) \
  -D __TRUSTINSOFT_NO_MTHREAD__ \
  -D __TRUSTINSOFT_DEBUG__ -D SOPC_DEBUG_PRINTING=true \
  -D TEST_TK_CLIENT \
  -val-ignore-recursive-calls -memexec-all \
  -no-val-malloc-returns-null \
  -val-stop-at-nth-alarm 1 \
  \
  -val-use-spec SOPC_Services_PreClear \
  -val-use-spec CryptoProvider_AsymEncrypt_RSA_OAEP \
  -val-use-spec CryptoProvider_AsymDecrypt_RSA_OAEP \
  -val-use-spec CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256 \
  -val-use-spec SOPC_ExtensionObject_Clear \
  -val-use-spec SOPC_NodeId_Clear \
  \
  -slevel-function Socket_Read:5000 \
  # \
  -slevel-function OpcUa_ResponseHeader_Decode:500000 \
  -slevel-function tis_memcpy_bounded:10000 \
  -slevel-function SOPC_Buffer_Read:5000 \
  -val-use-spec SOPC_ExtensionObject_Read \
  -val-use-spec SOPC_NodeId_Read \
  -slevel-function SOPC_MsgBodyType_Read:10000 \
  -slevel-function SOPC_ExtensionObject_Read:10000 \
  -slevel-function SC_ClientTransition_ScConnecting_To_ScConnected:5000 \
  -val-use-spec SOPC_Buffer_Read \

#-------------------------------------------------------------------------------
TEST_SCH_CLIENT_SRC:= \
  $(SRC_DIR)/tests/secure_channels/test_secure_channels_client.c \
  $(SRC_DIR)/tests/secure_channels/stubs/stub_sc_sopc_services_api.c \
  $(SRC_DIR)/tests/secure_channels/stubs/stub_sc_sopc_toolkit_config.c \
  $(RES)/sch_client_inputs.c \
  $(TIS_SRC_DIR)/stubs.c \
  $(TIS_SRC_DIR)/tis_time.c \
  $(shell find $(SRC_DIR)/csrc/opcua_types -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/helpers -type f -name "*.c") \
  $(wildcard $(SRC_DIR)/csrc/helpers_platform_dep/*.c) \
  $(shell find $(SRC_DIR)/csrc/helpers_platform_dep/linux -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/sockets -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/crypto -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/secure_channels -type f -name "*.c") \
  $(SRC_DIR)/csrc/configuration/sopc_ieee_check.c \
  # \


$(RES)/sch_client_inputs.c: $(TIS_DIR)/scripts/tis-inputs.awk
$(RES)/sch_client_inputs.c: $(RES)/test_secure_channels_client.log
	$(ECHO_CIBLE)
	gawk -f $(TIS_DIR)/scripts/tis-inputs.awk $< > $@

$(RES)/sch_client.state: $(TEST_SCH_CLIENT_SRC) \
                       $(TIS_SRC_DIR)/mbedtls_stubs.c \
                       $(TIS_SRC_DIR)/mbedtls_spec.acsl \

$(RES)/sch_client.state: TEST_SRCS += $(TEST_SCH_CLIENT_SRC) \

$(RES)/sch_client.state: TEST_ARGS += \
  $(TIS_ARGS_WITHOUT_MBEDTLS) \
  -I $(SRC_DIR)/tests/secure_channels/stubs \
  $(TIS_SRC_DIR)/tis_main.c -main tis_main \
  \
  -D __TRUSTINSOFT_NO_MTHREAD__ \
  -D TEST_SCH_CLIENT \
  -D __TRUSTINSOFT_DEBUG__ -D SOPC_DEBUG_PRINTING=true \
  -val-stop-at-nth-alarm 1 \
  -D SOPC_MAX_DIAG_INFO_NESTED_LEVEL=3 -val-clone-on-recursive-calls \
  -no-val-malloc-returns-null \
  -val-use-spec CryptoProvider_AsymEncrypt_RSA_OAEP \
  -val-use-spec CryptoProvider_AsymDecrypt_RSA_OAEP \
  -val-use-spec CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256 \
  -val-use-spec SOPC_ExtensionObject_Read \
  -val-use-spec SOPC_ExtensionObject_Clear \
  -val-use-spec SOPC_NodeId_Read \
  -val-use-spec SOPC_NodeId_Clear \
  -slevel-function tis_memcpy_bounded:4999 \
  -slevel-function SOPC_Buffer_Read:2000 \
  -slevel-function Socket_Read:5000 \
  -slevel-function SOPC_MsgBodyType_Read:10000 \
  -slevel-function SOPC_ExtensionObject_Read:10000 \
  -slevel-function SC_ClientTransition_ScConnecting_To_ScConnected:5000
  # \
  -val-ignore-recursive-calls -memexec-all \
  -slevel-function SOPC_NodeId_Clear:10000 \
  -memexec-all \
  -slevel-function CryptoProvider_AsymEncrypt_RSA_OAEP:10000 \
  -val-use-spec SOPC_Services_PreClear \

#-------------------------------------------------------------------------------
TEST_SOCKETS_SRC:= \
  $(SRC_DIR)/tests/sockets/check_sockets.c \
  $(SRC_DIR)/tests/sockets/stubs/stub_sockets_sopc_secure_channels_api.c \
  $(TIS_SRC_DIR)/tis_check_lib.c \
  $(TIS_SRC_DIR)/stubs.c \
  $(shell find $(SRC_DIR)/csrc/sockets -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/helpers -type f -name "*.c") \
  $(shell find $(SRC_DIR)/csrc/helpers_platform_dep/linux -type f -name "*.c") \

$(RES)/sockets.state $(RES)/ti_sockets.state: $(TEST_SOCKETS_SRC)
$(RES)/sockets.state $(RES)/ti_sockets.state: \
  TEST_SRCS += $(TEST_SOCKETS_SRC)

$(RES)/sockets.state: TEST_ARGS += \
  -I $(SRC_DIR)/tests/sockets/stubs \
  -D TEST_SOCKETS \
  -D __TRUSTINSOFT_NO_MTHREAD__ \
  -D __TRUSTINSOFT_DEBUG__ -D SOPC_DEBUG_PRINTING=true \
  -val-stop-at-nth-alarm 1 \
  -slevel-function test_sockets_fn:10000 \
  # \
  -no-val-malloc-returns-null \

$(RES)/ti_sockets.state: TEST_TI_ARGS += \
  --cc -I$(SRC_DIR)/tests/sockets/stubs \
  --cc -DTEST_SOCKETS \
  --cc -D__TRUSTINSOFT_NO_MTHREAD__ \
  --cc -D__TRUSTINSOFT_DEBUG__ --cc -DSOPC_DEBUG_PRINTING=true \

#-------------------------------------------------------------------------------

.PHONY: empty
empty: $(RES)/empty.state
BASES+=$(RES)/empty
$(RES)/empty.state: $(ALL_SRC_FILES) \
	            $(TIS_SRC_DIR)/tis_check_lib.c \
	            $(TIS_SRC_DIR)/stubs.c
	source $(TOOLS_DIR)/run.sh; TOOLS_DIR=$(TOOLS_DIR) \
	  run_analyzer "$(TIS_ARGS)" \
	  --custom-libc $(TIS_SRC_DIR)/stubs.c \
	  $(TIS_SRC_DIR)/tis_check_lib.c \
	  $(ALL_SRC_FILES) \
	  -acsl-import $(TIS_SRC_DIR)/empty.acsl \
	  -save $(RES)/empty.state > $(RES)/empty.log

GOALS:= -then -wp-fct SOPC_Buffer_Read -wp-prop wp

GOALS_cast+= \
	     -then -wp-fct SOPC_Read_Array -wp-prop wp \
	     -then -wp-fct SOPC_Buffer_Read -wp-prop wp \
	     -then -wp-fct SOPC_Buffer_Write -wp-prop wp \
	     -then -wp-fct SOPC_SLinkedList_Append -wp-prop wp \

.PHONY: wp
wp: $(RES)/wp_wpproperties.ok
BASES+=$(RES)/wp
$(RES)/wp.state: $(RES)/empty.state $(TIS_SRC_DIR)/mbedtls_spec.acsl \
	         $(TIS_DIR)/Makefile
	$(ECHO_CIBLE)
	$(ECHO_CIBLE)
	(source $(TOOLS_DIR)/run.sh; run_analyzer "$(TIS_LOAD_ARGS) \
	  -load $(RES)/empty.state \
	  -acsl-import $(TIS_SRC_DIR)/mbedtls_spec.acsl \
	  $(GOALS) \
	  -save $@.tmp" ; \
	  run_analyzer "$(TIS_LOAD_ARGS) -load $@.tmp \
	  -wp-model Typed+cast \
	  $(GOALS_cast) \
	  -then -tis-info-csv-all $(RES)/wp -save $@" ; \
	) > $(RES)/wp.log
	rm -f $@.tmp

GENERATED += $(addprefix $(RES)/wp, .log .state .db)
GENERATED += $(addprefix $(RES)/wp_wpproperties, .log .ok)
GENERATED += $(wildcard $(RES)/wp_*.csv)
.PRECIOUS: $(addprefix $(RES)/wp_wpproperties, .log .oracle .ok)

#-------------------------------------------------------------------------------
# Verifications:

COV_OPTIONS:=--nb-tests
COV_OPTIONS+=--skip-function "^__fc_"
COV_OPTIONS+=--skip-function "^__tis_"
COV_OPTIONS+=--skip-function "^mbedtls_"
COV_OPTIONS+=--unreachable-fun

CHECK_OPTIONS+= --no-top-dead-called
CHECK_OPTIONS+= --no-top-dead-not-called
CHECK_OPTIONS+= --no-top-dead-address
CHECK_OPTIONS+= --no-top-dead-indirect
CHECK_OPTIONS+= --no-coverage

CHECK_OPTIONS+= --skip-log "found two contracts for fcntl"
CHECK_OPTIONS+= --skip-log "found two contracts for gmtime"
CHECK_OPTIONS+= --skip-log "found two contracts for tis_memcpy_bounded"

CHECK_OPTIONS+= --skip-log "def'n of func gettimeofday"
CHECK_OPTIONS+= --skip-log "def'n of func tis_force_value"

CHECK_OPTIONS+= --skip-log "\[tis-input]"
CHECK_OPTIONS+= --skip-log "recursive call during value analysis"
CHECK_OPTIONS+= --skip-log "this seems innocuous"
CHECK_OPTIONS+= --skip-log "escaping the scope of a block"
CHECK_OPTIONS+= --skip-log "ignoring non-existing function 'atoq'"
CHECK_OPTIONS+= --skip-log "no assigns clauses for function .*printf"
CHECK_OPTIONS+= --skip-log "no assigns clauses for function .*scanf"
CHECK_OPTIONS+= --skip-log "ignoring missing assigns for all variadic functions"
CHECK_OPTIONS+= --skip-log "locals {scConfig} escaping the scope of main"

CHECK_OPTIONS+= --skip-props "_wp"
CHECK_OPTIONS+= --skip-props "accept:NONAME"
CHECK_OPTIONS+= --skip-props "strncpy:.*"
CHECK_OPTIONS+= --skip-props "strtoul:.*"
CHECK_OPTIONS+= --skip-props "allocates"
CHECK_OPTIONS+= --skip-props "assigns" --skip-props "from"

CHECK_OPTIONS+= --skip-lib "mbedtls_[a-z_0-9]*"
CHECK_OPTIONS+= --skip-lib __builtin_unreachable

CHECK_OPTIONS+= --skip-abort "_ck_assert_failed"
CHECK_OPTIONS+= --skip-abort "__FC_assert"
CHECK_OPTIONS+= --skip-abort "exit"
# now that non-terminaison is reported for each state, the preconditions
# of tis_memcpy_bounded may be invalid in some of them.
# CHECK_OPTIONS+= --skip-abort "tis_memcpy_bounded"
# CHECK_OPTIONS+= --skip-abort "tis_force_value"

PROJECT:=ingopcs

.PHONY: modifications
modifications: diff modif diff_files

# UPSTREAM_BRANCH=tags/INGOPCS_Toolkit_0.5.0
UPSTREAM_BRANCH=origin/tis-master
MODIF_OPTIONS:= --add INTERPRETER
MODIF_OPTIONS+= --add FIXED_MALLOC
# __TRUSTINSOFT_DEBUG__ defined for compilation, to add trace to real execution
MODIF_OPTIONS+= --add DEBUG
MODIF_OPTIONS+= --add SKIP_CRYPTO_TESTS
MODIF_OPTIONS+= --add NO_MTHREAD
MODIF_OPTIONS+= --add NO_MBEDTLS
MODIF_OPTIONS+= --add PATCH

.PHONY: diff
diff: $(RES)/$(PROJECT).src_diff.ok
GENERATED += $(addprefix $(RES)/$(PROJECT).src_diff, .log .ok)
$(RES)/$(PROJECT).src_diff.log: $(STACK_SRC_FILES) $(SRC_DIR) $(TIS_DIR)/Makefile
.PRECIOUS: $(addprefix $(RES)/$(PROJECT).src_diff, .log .oracle)

.PHONY: modif
modif: $(RES)/$(PROJECT).src_modif.ok
GENERATED += $(addprefix $(RES)/$(PROJECT).src_modif, .log .ok)
$(RES)/$(PROJECT).src_modif.log: $(STACK_SRC_FILES) $(SRC_DIR) $(TIS_DIR)/Makefile
.PRECIOUS: $(addprefix $(RES)/$(PROJECT).src_modif, .log .oracle)

.PHONY: diff_files
diff_files: $(RES)/$(PROJECT).diff_other_files.ok
$(RES)/$(PROJECT).diff_files.log: $(SRC_DIR) Makefile
GENERATED += $(addprefix $(RES)/$(PROJECT).diff_files, .log)
$(RES)/$(PROJECT).diff_other_files.log: \
                          $(RES)/$(PROJECT).diff_files.log
	cat $< \
	  | grep -v "[ch]$$" \
	  | grep -v tis-analysis \
	  > $@
GENERATED += $(addprefix $(RES)/$(PROJECT).diff_other_files, .log .ok)
.PRECIOUS: $(addprefix $(RES)/$(PROJECT).diff_other_files, .log .oracle .ok)

#-------------------------------------------------------------------------------
# Coverage:

.PHONY: cov
cov: $(RES)/$(PROJECT)_coverage.ok

COV_OPTIONS+=--unreachable-fun
COV_OPTIONS+=--skip-function "^main"
COV_OPTIONS+=--skip-function "^tis_"
COV_OPTIONS+=--skip-function "^test[s]?_"
COV_OPTIONS+=--skip-function "^tcase_"
COV_OPTIONS+=--skip-function "^tlibw_"
COV_OPTIONS+=--skip-function "^suite_"
COV_OPTIONS+=--skip-function "^srunner_"
COV_OPTIONS+=--skip-function "^teardown_"

# Consider having a $(PROJECT)_coverage.justif file.
# COV_OPTIONS+=--justif-coverage $(PROJECT)_coverage.justif
# $(RES)/$(PROJECT)_coverage.log: $(PROJECT)_coverage.justif

# To group several analyses to get the overall project coverage:
$(RES)/$(PROJECT)_multi.log: $(TEST_NAMES:%=$(RES)/%_kitgen.log)
	$(ECHO_CIBLE)
	cat $^ > $@

# GENERATED+=$(RES)/$(PROJECT)_multi.log
GENERATED+=$(TEST_NAMES:%=$(RES)/%_kitgen.log)
GENERATED += $(RES)/$(PROJECT)_multi.log
GENERATED += $(addprefix $(RES)/$(PROJECT)_coverage, .log .ok)
.PRECIOUS: $(addprefix $(RES)/$(PROJECT)_coverage, .log .oracle .ok)

html_cov: $(RES)/$(PROJECT)_coverage.html

$(RES)/$(PROJECT)_coverage.html: $(TOOLS_DIR)/html-coverage
$(RES)/$(PROJECT)_coverage.html: $(RES)/cov_statements.csv
	$(ECHO_CIBLE)
	$(TOOLS_DIR)/html-coverage $< > $@

$(RES)/cov_statements.csv: $(BASES:%=%_statements.csv)
	$(ECHO_CIBLE)
	# TODO: add more filters
	tail --quiet --lines=+2 $^ \
	  | grep -v TIS_KERNEL_SHARE \
	  | grep -v "/tis-analysis" \
	  > $@

$(BASES:%=%_statements.csv): %_statements.csv: %.db


#-------------------------------------------------------------------------------
# Cleaning:

GENERATED += $(foreach base, $(BASES), \
                       $(addprefix $(base), .log .state .db) \
                       $(addprefix $(base), _check.log _check.ok) \
                       $(wildcard $(base)_*.csv))

.PRECIOUS: $(foreach base, $(BASES), \
             $(addprefix $(base), .log .state .db _all.csv) \
             $(addprefix $(base), _check.log _check.oracle _check.ok))

clean:
	rm -rf $(GENERATED)

#===============================================================================

