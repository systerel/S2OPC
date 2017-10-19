PATHBMODEL=.
PATHGENTRAD=$(PATHBMODEL)/tradc
PATHTRAD=../csrc/services/bgenc

BMODEL_MCH_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.mch")
BMODEL_IMP_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.imp")
BMODEL_IMP_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.imp" -printf "%f\n")
BMODEL_DEF_PATH_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*.def")

BMODEL_MCH_BS_SRCS=$(shell find $(PATHBMODEL) -maxdepth 1 -name "*bs.mch" -printf "%f\n")

BMODEL_C_FILES=$(patsubst %_i.imp,$(PATHTRAD)/%.c,$(BMODEL_IMP_SRCS))

BMODEL_H_FILES_NOT_FILTERED=$(patsubst %_i.imp,$(PATHTRAD)/%.h,$(BMODEL_IMP_SRCS))
BMODEL_H_FILES_NOT_FILTERED+=$(patsubst %_bs.mch,$(PATHTRAD)/%_bs.h,$(BMODEL_MCH_BS_SRCS))
BMODEL_H_FILES=$(filter-out $(PATHTRAD)/constants_bs.h, $(BMODEL_H_FILES_NOT_FILTERED))

ADDITIONAL_FILES=$(PATHTRAD)/toolkit_header_init.c $(PATHTRAD)/toolkit_header_init.h

all: $(BMODEL_C_FILES) $(BMODEL_H_FILES) $(ADDITIONAL_FILES)

# All generated C files (*.c/*.h) from B model
$(BMODEL_C_FILES) $(BMODEL_H_FILES) $(PATHTRAD)/toolkit_header_init.c $(PATHTRAD)/toolkit_header_init.h: gen.intermediate

# Indicates gen.intermediate is never present since it is an intermediate
.INTERMEDIATE: gen.intermediate

gen.intermediate: $(BMODEL_MCH_PATH_SRCS) $(BMODEL_IMP_PATH_SRCS) $(BMODEL_IMP_PATH_SRCS)
	@\rm -f ./bdp/*.nf
	@m -tc -b0 > tc.log
	@grep -iEc '(warning|B0Check)' tc.log | xargs test 0 -eq
	@\rm tc.log
	@\rm -fr ./$(PATHTRAD)
	@\rm -fr ./$(PATHGENTRAD)
	@m -tr > tr.log
	@grep -iEc '(warning|error)' tr.log | xargs test 0 -eq
	@\rm tr.log
	@mkdir $(PATHTRAD)
	@cp $(PATHGENTRAD)/*.c $(PATHGENTRAD)/*.h $(PATHTRAD)
	@# Renames unwanted files, such as the ones that define a void * to a t_entier4
	@for i in constants_bs.h; do \
	    mv $(PATHTRAD)/"$$i" $(PATHTRAD)/"$$i"_; \
	done
