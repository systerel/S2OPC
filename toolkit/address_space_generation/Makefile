# Old makefile, ct.py moved to tooling

CC=gcc
CFLAGS=-W -Wall -ansi -pedantic -std=c11 -I./install/include/
LDFLAGS=
EXEC=test
XSL=test.xsl add.xsl
SRC=$(XSL:.xsl=.c)
OBJ=$(SRC:.c=.o)

PATHBUILD=./build
PATHGENC=./genc

.PHONY: all config clean

all: config $(EXEC)

config:
	@mkdir -p $(PATHBUILD)
	@mkdir -p $(PATHGENC)

clean:
	@rm -r ./$(PATHBUILD)
	@rm -r ./$(PATHGENC)

$(PATHGENC)/%.c : %.xsl
	ct.py $< $(PATHBUILD)/$<
	saxonb-xslt -xsl:$(PATHBUILD)/$< -s:testView.xml -o:$@

$(PATHBUILD)/%.o: $(PATHGENC)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(PATHBUILD)/$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)


