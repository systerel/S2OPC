# Old makefile, ct.py moved to tooling

empty:=
space:= $(empty) $(empty)
CC=gcc
CFLAGS=-W -Wall -ansi -pedantic -std=c99 -I./ -I../src/impl -I../../install_stack/include/
LDFLAGS=
EXEC=test
XSL=add.xsl test.xsl
SRC=$(XSL:.xsl=.c)
OBJ=$(SRC:.c=.o)
OBJ_WITH_PATH=$(PATHBUILD)/$(subst $(space), $(PATHBUILD)/,$(OBJ))
SRC_WITH_PATH=$(PATHGENC)/$(subst $(space), $(PATHGENC)/,$(SRC))
PATHBUILD=./build
PATHGENC=./genc

.SECONDARY:$(SRC_WITH_PATH)
.PHONY: all config clean cleangen

all: config $(EXEC)

config:
	@mkdir -p $(PATHBUILD)
	@mkdir -p $(PATHGENC)

clean:
	@echo "clean directory"
	@rm -f $(EXEC)
	@rm -rf $(PATHBUILD)

cleangen:
	@rm -rf $(PATHGENC)

$(PATHGENC)/%.c : %.xsl
	ct.py $< $(PATHBUILD)/$<
	saxonb-xslt -xsl:$(PATHBUILD)/$< -s:testView.xml -o:$@

$(PATHBUILD)/%.o: $(PATHGENC)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJ_WITH_PATH)
	$(CC) -o $@ $^ $(LDFLAGS)


