SRC_SUFFIX = c
LD = $(CC)
COMPILE.flags.c = $(CFLAGS) $(COMPILE.flags)
COMPILE.c = $(CC) $(COMPILE.flags.c) -c
ifndef STATIC_LINK
COMPILE.c += -fPIC
endif

libflags = -L$(OUTLIB_DIR) -Wl,-rpath,$(abspath $(OUTLIB_DIR)) -Wl,-rpath,'$$ORIGIN/../lib' -l$(PROJECT)
BUILD_bin.c = $(CC) $(COMPILE.flags.c) -o $@ $< $(libflags) $(LINK.flags)
BUILD_test_bin.c = $(BUILD_bin.c) -UNDEBUG

ROOT_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
include $(addprefix $(ROOT_DIR)/,common.mk)

$(OBJ_DIR)/%.$(SRC_SUFFIX).o : %.$(SRC_SUFFIX) $(DEP_DIR)/%.$(SRC_SUFFIX).d $(OUT_REV)
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.$(SRC_SUFFIX).d)"
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(TARGET_BINS) : $(OUTBIN_DIR)/% : %.$(SRC_SUFFIX) $(DEP_DIR)/%.$(SRC_SUFFIX).d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.$(SRC_SUFFIX).d)"
	$(BUILD_bin.c)
	$(POSTCOMPILE)

$(TARGET_TESTS) : $(OBJ_DIR)/% : %.$(SRC_SUFFIX) $(DEP_DIR)/%.$(SRC_SUFFIX).d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.$(SRC_SUFFIX).d)"
	$(BUILD_test_bin.c)
	$(POSTCOMPILE)
