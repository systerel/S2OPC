# Old makefile, ct.py moved to tooling

empty:=
space:= $(empty) $(empty)
CC=gcc
CFLAGS=-g3 -W -Wall -ansi -pedantic -std=c99 -I./ -I../src/impl -I../../install_stack/include/
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
.PHONY: all config clean cleangen cleanall

all: config $(EXEC) $(PATHGENC)/test_read_write_addspace.c

config:
	@echo "configure directories"
	@mkdir -p $(PATHBUILD)
	@mkdir -p $(PATHGENC)

clean:
	@echo "clean build directory and execs"
	@rm -f $(EXEC)
	@rm -rf $(PATHBUILD)

cleangen:
	@echo "clean generated C files directory"
	@rm -rf $(PATHGENC)

cleanall: clean cleangen

$(PATHGENC)/%.c : %.xsl testView.xml
	./scripts/ct.py $< $(PATHBUILD)/$<
	saxonb-xslt -xsl:$(PATHBUILD)/$< -s:testView.xml -o:$@

$(PATHGENC)/test_%_addspace.c : test_%.xml add.xsl
	./scripts/ct.py add.xsl $(PATHBUILD)/add.xsl
	saxonb-xslt -xsl:$(PATHBUILD)/add.xsl -s:$< -o:$@

$(PATHBUILD)/%.o: $(PATHGENC)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJ_WITH_PATH)
	$(CC) -o $@ $^ $(LDFLAGS)


