STACK_PATH=./stack
LIB_STACK_PATH=$(STACK_PATH)/install/lib
LIB_STACK_FILE=libingopcs.a

TOOLKIT_PATH=./toolkit

.PHONY: all stack toolkit doc clean cleanall test

default: all

all: stack toolkit

stack: $(LIB_STACK_PATH)/$(LIB_STACK_FILE)

$(LIB_STACK_PATH)/$(LIB_STACK_FILE): 
	@make -C $(STACK_PATH) all

toolkit:
	@make -C $(TOOLKIT_PATH) all

clean:
	@make -C $(STACK_PATH) clean
	@make -C $(TOOLKIT_PATH) clean

cleanall:
	@make -C $(STACK_PATH) cleanall
	@make -C $(TOOLKIT_PATH) clean
