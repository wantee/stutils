CXX_SUFFIX = cc
LD = $(CXX)
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
ifndef STATIC_LINK
COMPILE.cc += -fPIC
endif
COMPILE_bin.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< -L$(OUTLIB_DIR) -l$(PROJECT) $(LDFLAGS)
COMPILE_test_bin.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -UNDEBUG -o $@ $< -L$(OUTLIB_DIR) -l$(PROJECT) $(LDFLAGS)
ifdef STATIC_LINK
COMPILE_bin.cc += -L$(OBJ_DIR) -Wl,-rpath,'$$ORIGIN/../lib'
COMPILE_test_bin.cc += -L$(OBJ_DIR)
else
COMPILE_bin.cc += -L$(OUTLIB_DIR) -Wl,-rpath,$(abspath $(OUTLIB_DIR)) -Wl,-rpath,'$$ORIGIN/../lib'
COMPILE_test_bin.cc += -L$(OUTLIB_DIR) -Wl,-rpath,$(abspath $(OUTLIB_DIR))
endif

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
	$(COMPILE_bin.cc)
	$(POSTCOMPILE)

$(TARGET_TESTS) : $(OBJ_DIR)/% : %.$(CXX_SUFFIX) $(DEP_DIR)/%.d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(COMPILE_test_bin.cc)
	$(POSTCOMPILE)
