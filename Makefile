STACK_PATH=./stack
LIB_STACK_PATH=$(STACK_PATH)/install/lib
LIB_STACK_FILE=libingopcs.a
TOOLKIT_PATH=./toolkit

BIN_DIR=bin
BIN_PATH=../$(BIN_DIR)

INSTALL_STACK_DIR=install_stack
INSTALL_STACK_PATH=../$(INSTALL_STACK_DIR)

.PHONY: all stack toolkit doc clean cleanall test

default: all

all: stack toolkit

stack: $(LIB_STACK_PATH)/$(LIB_STACK_FILE)

$(LIB_STACK_PATH)/$(LIB_STACK_FILE): 
	@make -C $(STACK_PATH) LOCAL_INSTALL_DIR=$(INSTALL_STACK_PATH) EXEC_DIR=$(BIN_PATH) all

toolkit:
	@make -C $(TOOLKIT_PATH) PATHEXEC=$(BIN_PATH) all

clean:
	@rm -fr $(BIN_DIR)
	@rm -fr $(INSTALL_STACK_DIR)
	@make -C $(STACK_PATH) clean
	@make -C $(TOOLKIT_PATH) clean

cleanall: clean
	@make -C $(STACK_PATH) cleanall
