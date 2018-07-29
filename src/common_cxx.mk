SRC_SUFFIX = cc
LD = $(CXX)
COMPILE.flags.cc = $(CXXFLAGS) $(COMPILE.flags)
COMPILE.cc = $(CXX) $(COMPILE.flags.cc) -c

libflags = -L$(OUTLIB_DIR) -l$(PROJECT)
ifndef STATIC_LINK
  COMPILE.cc += -fPIC
  libflags += -Wl,-rpath,$(abspath $(OUTLIB_DIR)) -Wl,-rpath,'$$ORIGIN/../lib'
endif
BUILD_bin.cc = $(CXX) $(COMPILE.flags.cc) -o $@ $< $(libflags) $(LINK.flags)
BUILD_test_bin.cc = $(BUILD_bin.cc) -UNDEBUG

ROOT_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
include $(addprefix $(ROOT_DIR)/,common.mk)

$(OBJ_DIR)/%.$(SRC_SUFFIX).o : %.$(SRC_SUFFIX) $(DEP_DIR)/%.$(SRC_SUFFIX).d $(OUT_REV)
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.$(SRC_SUFFIX).d)"
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(TARGET_BINS) : $(OUTBIN_DIR)/% : %.$(SRC_SUFFIX) $(DEP_DIR)/%.$(SRC_SUFFIX).d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.$(SRC_SUFFIX).d)"
	$(BUILD_bin.cc)
	$(POSTCOMPILE)

$(TARGET_TESTS) : $(OBJ_DIR)/% : %.$(SRC_SUFFIX) $(DEP_DIR)/%.$(SRC_SUFFIX).d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.$(SRC_SUFFIX).d)"
	$(BUILD_test_bin.cc)
	$(POSTCOMPILE)
