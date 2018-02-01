LD = $(CC)
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
ifndef STATIC_LINK
COMPILE.c += -fPIC
endif
COMPILE_bin.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $< -l$(PROJECT) $(LDFLAGS)
COMPILE_test_bin.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -UNDEBUG -o $@ $<  -L$(OUTLIB_DIR) -l$(PROJECT) $(LDFLAGS)
ifdef STATIC_LINK
COMPILE_bin.c += -L$(OBJ_DIR) -Wl,-rpath,'$$ORIGIN/../lib'
COMPILE_bin.cc += -L$(OBJ_DIR) -Wl,-rpath,'$$ORIGIN/../lib'
else
COMPILE_bin.c += -L$(OUTLIB_DIR) -Wl,-rpath,$(abspath $(OUTLIB_DIR)) -Wl,-rpath,'$$ORIGIN/../lib'
COMPILE_bin.cc += -L$(OUTLIB_DIR) -Wl,-rpath,$(abspath $(OUTLIB_DIR)) -Wl,-rpath,'$$ORIGIN/../lib'
endif

ROOT_DIR = $(dir $(lastword $(MAKEFILE_LIST)))
include $(addprefix $(ROOT_DIR)/,common.mk)

$(OBJ_DIR)/%.o : %.c $(DEP_DIR)/%.d $(OUT_REV)
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(TARGET_BINS) : $(OUTBIN_DIR)/% : %.c $(DEP_DIR)/%.d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(COMPILE_bin.c)
	$(POSTCOMPILE)

$(TARGET_TESTS) : $(OBJ_DIR)/% : %.c $(DEP_DIR)/%.d
	@mkdir -p "$(dir $@)"
	@mkdir -p "$(dir $(DEP_DIR)/$*.d)"
	$(COMPILE_test_bin.c)
	$(POSTCOMPILE)
