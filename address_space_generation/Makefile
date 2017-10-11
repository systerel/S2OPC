PATHBUILD=./build
PATHGENC=./genc

XMLPATH=../tests/data/address_space
XMLFILES=$(shell find $(XMLPATH) -name "*.xml")
CFILES=$(patsubst $(XMLPATH)/%.xml,$(PATHGENC)/%_addspace.c,$(XMLFILES))
CTESTFILES=$(patsubst $(XMLPATH)/%.xml,$(PATHGENC)/check_%_addspace.c,$(XMLFILES))

.PHONY: all config clean

all: config $(CFILES) $(CTESTFILES)

config:
	@echo "Configure directories"
	@mkdir -p $(PATHBUILD)
	@mkdir -p $(PATHGENC)

clean:
	@echo "Clean build and generated C files directories"
	@rm -rf $(PATHBUILD)
	@rm -rf $(PATHGENC)

$(PATHGENC)/%_addspace.c: $(XMLPATH)/%.xml gen_address_space.xsl
	@echo "Generating address space $@ from $^"
	@./scripts/ct.py gen_address_space.xsl $(PATHBUILD)/gen_address_space.xsl
	@saxonb-xslt -xsl:$(PATHBUILD)/gen_address_space.xsl -s:$< -o:$@

$(PATHGENC)/check_%_addspace.c: $(XMLPATH)/%.xml gen_check_address_space.xsl
	@echo "Generating address space check test $@ from $^"
	@./scripts/ct.py gen_check_address_space.xsl $(PATHBUILD)/gen_check_address_space.xsl
	@saxonb-xslt -xsl:$(PATHBUILD)/gen_check_address_space.xsl -s:$< -o:$@
