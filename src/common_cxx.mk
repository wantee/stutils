CXX_SUFFIX = cc
LD = $(CXX)
COMPILE.flags.cc = $(CXXFLAGS) $(COMPILE.flags)
COMPILE.cc = $(CXX) $(COMPILE.flags.cc) -c
ifndef STATIC_LINK
COMPILE.cc += -fPIC
endif

libflags = -L$(OUTLIB_DIR) -Wl,-rpath,$(abspath $(OUTLIB_DIR)) -Wl,-rpath,'$$ORIGIN/../lib' -l$(PROJECT)
BUILD_bin.cc = $(CXX) $(COMPILE.flags.cc) -o $@ $< $(libflags) $(LINK.flags)
BUILD_test_bin.cc = $(BUILD_bin.cc) -UNDEBUG

ROOT_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
include $(addprefix $(ROOT_DIR)/,common.mk)

$(OBJ_DIR)/%.o : %.$(CXX_SUFFIX) $(DEP_DIR)/%.d $(OUT_REV)
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(TARGET_BINS) : $(OUTBIN_DIR)/% : %.$(CXX_SUFFIX) $(DEP_DIR)/%.d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(BUILD_bin.cc)
	$(POSTCOMPILE)

$(TARGET_TESTS) : $(OBJ_DIR)/% : %.$(CXX_SUFFIX) $(DEP_DIR)/%.d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(BUILD_test_bin.cc)
	$(POSTCOMPILE)
