PATHBUILD=./build
PATHGENC=./genc
PATHSCRIPT=./scripts

XMLPATH=../tests/data/address_space
XMLFILES=$(shell find $(XMLPATH) -name "*.xml" | grep -v "/parts/")
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

$(PATHGENC)/%_addspace.c: $(XMLPATH)/%.xml $(PATHSCRIPT)/gen_address_space.xsl common_part/Common_Address_Space.xml
	@echo "Generating address space $@ from $^"
	@$(PATHSCRIPT)/gen_ingopcs_address_space.sh $(XMLPATH)/parts/User_Address_Space.xml $(XMLPATH)/ingopcs.xml
	@xmllint -schema spec/UANodeSet.xsd -noout $<
	@$(PATHSCRIPT)/ct.py $(PATHSCRIPT)/gen_address_space.xsl $(PATHBUILD)/gen_address_space.xsl
	@saxonb-xslt -xsl:$(PATHBUILD)/gen_address_space.xsl -s:$< -o:$@

$(PATHGENC)/check_%_addspace.c: $(XMLPATH)/%.xml $(PATHSCRIPT)/gen_check_address_space.xsl common_part/Common_Address_Space.xml
	@echo "Generating address space check test $@ from $^"
	@xmllint -schema spec/UANodeSet.xsd -noout $<
	@./scripts/ct.py $(PATHSCRIPT)/gen_check_address_space.xsl $(PATHBUILD)/gen_check_address_space.xsl
	@saxonb-xslt -xsl:$(PATHBUILD)/gen_check_address_space.xsl -s:$< -o:$@
