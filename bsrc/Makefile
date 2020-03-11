PATHBMODEL=.
PATHGENTRAD=$(PATHBMODEL)/tradc
PATHTRAD=../src/ClientServer/services/bgenc

BMODEL_MCH_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.mch")
BMODEL_IMP_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.imp")
BMODEL_IMP_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.imp" -printf "%f\n")
BMODEL_DEF_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.def")
BMODEL_REF_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.ref")

BMODEL_MCH_BS_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*bs.mch" -printf "%f\n")

BMODEL_C_FILES=$(patsubst %_i.imp,$(PATHTRAD)/%.c,$(BMODEL_IMP_SRCS))

BMODEL_H_FILES_NOT_FILTERED=$(patsubst %_i.imp,$(PATHTRAD)/%.h,$(BMODEL_IMP_SRCS))
BMODEL_H_FILES_NOT_FILTERED+=$(patsubst %_bs.mch,$(PATHTRAD)/%_bs.h,$(BMODEL_MCH_BS_SRCS))
BMODEL_H_FILES=$(filter-out $(PATHTRAD)/constants_bs.h, $(BMODEL_H_FILES_NOT_FILTERED))

ADDITIONAL_FILES=$(PATHTRAD)/toolkit_header_init.c $(PATHTRAD)/toolkit_header_init.h

# B model source files list
SOURCE_FILES=$(BMODEL_MCH_PATH_SRCS) $(BMODEL_IMP_PATH_SRCS) $(BMODEL_IMP_PATH_SRCS) $(BMODEL_REF_PATH_SRCS) $(BMODEL_DEF_PATH_SRCS)

# Generated C files list
TARGET_GENERATED_FILES=$(BMODEL_C_FILES) $(BMODEL_H_FILES) $(ADDITIONAL_FILES)

all: $(TARGET_GENERATED_FILES)

ifndef VERBOSE
.SILENT:
endif

$(TARGET_GENERATED_FILES): $(SOURCE_FILES)
	m -remove || true
	m -c -a
	$(RM) -f ./bdp/*.nf
	m -lonely -incomplete -pchk -tc -b0 | tee tc.log
	grep -iEc '(\bwarning|\berror|\bfailed\b|\berrno\b)' tc.log | xargs test 0 -eq
	$(RM) tc.log
	$(RM) -fr ./$(PATHTRAD)
	$(RM) -fr ./$(PATHGENTRAD)
	m -tr | tee tr.log
	grep -iEc '(\bwarning|\berror|\bfailed\b|\berrno\b)' tr.log | xargs test 0 -eq
	$(RM) tr.log
#	# Copy files and prepend them with copyright
	mkdir $(PATHTRAD)
	cd $(PATHGENTRAD); find . -name "*.[hc]" -exec ../prependcopyright.sh {} ../$(PATHTRAD)/{} \;
#	# Renames unwanted files, such as the ones that define a void * to a t_entier4
	for i in constants_bs.h; do \
	    mv $(PATHTRAD)/"$$i" $(PATHTRAD)/"$$i"_; \
	done
